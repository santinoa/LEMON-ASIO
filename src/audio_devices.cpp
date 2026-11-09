#include "audio_devices.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propsys.h>
#include <wrl/client.h>
#include <propvarutil.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

using Microsoft::WRL::ComPtr;

namespace {

std::wstring HResultText(HRESULT hr) {
    std::wostringstream stream;
    stream << L"HRESULT 0x" << std::hex << std::uppercase
           << static_cast<unsigned long>(hr);
    return stream.str();
}

std::wstring ReadFriendlyName(IMMDevice* device) {
    ComPtr<IPropertyStore> store;
    if (FAILED(device->OpenPropertyStore(STGM_READ, &store))) {
        return L"Dispositivo de audio (sin nombre)";
    }

    PROPVARIANT value;
    PropVariantInit(&value);
    std::wstring result = L"Dispositivo de audio (sin nombre)";

    if (SUCCEEDED(store->GetValue(PKEY_Device_FriendlyName, &value)) &&
        value.vt == VT_LPWSTR && value.pwszVal != nullptr) {
        result = value.pwszVal;
    }

    PropVariantClear(&value);
    return result;
}

std::wstring InspectDevice(IMMDevice* device) {
    std::wostringstream details;
    ComPtr<IAudioClient> audioClient;
    const HRESULT activateHr = device->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL, nullptr,
        reinterpret_cast<void**>(audioClient.GetAddressOf()));

    if (FAILED(activateHr)) {
        details << L"No se pudo consultar el formato del dispositivo.\r\n"
                << HResultText(activateHr);
        return details.str();
    }

    WAVEFORMATEX* mixFormat = nullptr;
    const HRESULT formatHr = audioClient->GetMixFormat(&mixFormat);
    if (SUCCEEDED(formatHr) && mixFormat != nullptr) {
        details << L"Formato compartido de Windows: "
                << mixFormat->nSamplesPerSec << L" Hz, "
                << mixFormat->nChannels << L" canales, "
                << mixFormat->wBitsPerSample << L" bits";

        if (mixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
            details << L" (float)";
        } else if (mixFormat->wFormatTag == WAVE_FORMAT_PCM) {
            details << L" (PCM)";
        } else if (mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
            details << L" (WAVEFORMATEXTENSIBLE)";
        }
        details << L"\r\n";
        CoTaskMemFree(mixFormat);
    } else {
        details << L"No se pudo obtener el formato de mezcla: "
                << HResultText(formatHr) << L"\r\n";
    }

    REFERENCE_TIME defaultPeriod = 0;
    REFERENCE_TIME minimumPeriod = 0;
    const HRESULT periodHr = audioClient->GetDevicePeriod(&defaultPeriod, &minimumPeriod);
    if (SUCCEEDED(periodHr)) {
        details << std::fixed << std::setprecision(2)
                << L"Periodo predeterminado del endpoint: "
                << (static_cast<double>(defaultPeriod) / 10000.0) << L" ms\r\n"
                << L"Periodo mínimo informado: "
                << (static_cast<double>(minimumPeriod) / 10000.0) << L" ms";
    } else {
        details << L"No se pudo consultar el periodo del dispositivo: "
                << HResultText(periodHr);
    }

    return details.str();
}

} // namespace

std::vector<AudioDeviceInfo> EnumerateAudioDevices(bool input, std::wstring& errorMessage) {
    errorMessage.clear();
    std::vector<AudioDeviceInfo> devices;
    const EDataFlow flow = input ? eCapture : eRender;

    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_ALL, IID_PPV_ARGS(&enumerator));
    if (FAILED(hr)) {
        errorMessage = L"No se pudo iniciar el enumerador de audio: " + HResultText(hr);
        return devices;
    }

    std::wstring defaultId;
    ComPtr<IMMDevice> defaultDevice;
    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(flow, eConsole, &defaultDevice))) {
        LPWSTR rawDefaultId = nullptr;
        if (SUCCEEDED(defaultDevice->GetId(&rawDefaultId)) && rawDefaultId != nullptr) {
            defaultId = rawDefaultId;
            CoTaskMemFree(rawDefaultId);
        }
    }

    ComPtr<IMMDeviceCollection> collection;
    hr = enumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &collection);
    if (FAILED(hr)) {
        errorMessage = input
            ? L"No se pudieron enumerar las entradas de audio: " + HResultText(hr)
            : L"No se pudieron enumerar las salidas de audio: " + HResultText(hr);
        return devices;
    }

    UINT count = 0;
    hr = collection->GetCount(&count);
    if (FAILED(hr)) {
        errorMessage = L"No se pudo leer la lista de dispositivos: " + HResultText(hr);
        return devices;
    }

    for (UINT index = 0; index < count; ++index) {
        ComPtr<IMMDevice> device;
        if (FAILED(collection->Item(index, &device))) {
            continue;
        }

        LPWSTR rawId = nullptr;
        if (FAILED(device->GetId(&rawId)) || rawId == nullptr) {
            continue;
        }

        AudioDeviceInfo info;
        info.id = rawId;
        CoTaskMemFree(rawId);
        info.name = ReadFriendlyName(device.Get());
        info.isDefault = (!defaultId.empty() && info.id == defaultId);
        info.details = InspectDevice(device.Get());
        devices.push_back(std::move(info));
    }

    return devices;
}
