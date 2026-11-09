#include "audio_devices.h"

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

namespace {

constexpr int ID_INPUT_COMBO = 1001;
constexpr int ID_OUTPUT_COMBO = 1002;
constexpr int ID_REFRESH = 1003;
constexpr int ID_BUFFER_COMBO = 1004;
constexpr int ID_RATE_COMBO = 1005;
constexpr int ID_INPUT_DETAILS = 1006;
constexpr int ID_OUTPUT_DETAILS = 1007;
constexpr int ID_STATUS = 1008;

HINSTANCE g_instance = nullptr;
HWND g_inputCombo = nullptr;
HWND g_outputCombo = nullptr;
HWND g_bufferCombo = nullptr;
HWND g_rateCombo = nullptr;
HWND g_inputDetails = nullptr;
HWND g_outputDetails = nullptr;
HWND g_status = nullptr;
HFONT g_font = nullptr;
HFONT g_titleFont = nullptr;
HBRUSH g_backgroundBrush = nullptr;
HBRUSH g_panelBrush = nullptr;
std::vector<AudioDeviceInfo> g_inputDevices;
std::vector<AudioDeviceInfo> g_outputDevices;

HWND CreateLabel(HWND parent, const wchar_t* text, int x, int y, int width, int height,
                 HFONT font = nullptr) {
    HWND label = CreateWindowExW(0, L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, width, height, parent, nullptr, g_instance, nullptr);
    SendMessageW(label, WM_SETFONT, reinterpret_cast<WPARAM>(font ? font : g_font), TRUE);
    return label;
}

void SetStatus(const std::wstring& text) {
    if (g_status) SetWindowTextW(g_status, text.c_str());
}

void UpdateDeviceDetails(bool input) {
    const HWND combo = input ? g_inputCombo : g_outputCombo;
    const HWND details = input ? g_inputDetails : g_outputDetails;
    const auto& devices = input ? g_inputDevices : g_outputDevices;
    const LRESULT selected = SendMessageW(combo, CB_GETCURSEL, 0, 0);

    if (selected == CB_ERR || static_cast<size_t>(selected) >= devices.size()) {
        SetWindowTextW(details, input
            ? L"No hay un micrófono seleccionado. Conectá el micrófono USB y actualizá la lista."
            : L"No hay auriculares seleccionados. Conectá los auriculares USB y actualizá la lista.");
        return;
    }

    const auto& device = devices[static_cast<size_t>(selected)];
    std::wstring text = device.name;
    if (device.isDefault) text += L"\r\nDispositivo predeterminado de Windows";
    text += L"\r\n\r\n" + device.details;
    SetWindowTextW(details, text.c_str());
}

void FillDeviceCombo(HWND combo, const std::vector<AudioDeviceInfo>& devices) {
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    for (const auto& device : devices) {
        std::wstring label = device.name;
        if (device.isDefault) label += L"  [Predeterminado]";
        SendMessageW(combo, CB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(label.c_str()));
    }

    if (devices.empty()) {
        return;
    }

    LRESULT defaultIndex = 0;
    for (size_t i = 0; i < devices.size(); ++i) {
        if (devices[i].isDefault) {
            defaultIndex = static_cast<LRESULT>(i);
            break;
        }
    }
    SendMessageW(combo, CB_SETCURSEL, defaultIndex, 0);
}

void RefreshDevices() {
    std::wstring inputError;
    std::wstring outputError;
    g_inputDevices = EnumerateAudioDevices(true, inputError);
    g_outputDevices = EnumerateAudioDevices(false, outputError);

    FillDeviceCombo(g_inputCombo, g_inputDevices);
    FillDeviceCombo(g_outputCombo, g_outputDevices);
    UpdateDeviceDetails(true);
    UpdateDeviceDetails(false);

    if (!g_inputDevices.empty() && !g_outputDevices.empty()) {
        SetStatus(L"Entrada y salida detectadas. Revisá que sean tus dispositivos USB.");
    } else if (g_inputDevices.empty() && g_outputDevices.empty()) {
        SetStatus(L"No se detectaron entradas ni salidas activas.");
        if (!inputError.empty() || !outputError.empty()) {
            const std::wstring error = L"Entrada: " + inputError + L"\r\nSalida: " + outputError;
            SetWindowTextW(g_inputDetails, error.c_str());
        }
    } else if (g_inputDevices.empty()) {
        SetStatus(L"Hay salidas, pero no se detectaron micrófonos activos.");
    } else {
        SetStatus(L"Hay entradas, pero no se detectaron salidas activas.");
    }
}

void AddComboItems(HWND combo, const std::vector<const wchar_t*>& items, int selected) {
    for (const auto* item : items) {
        SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item));
    }
    SendMessageW(combo, CB_SETCURSEL, selected, 0);
}

HWND CreateDetailsBox(HWND parent, int id, int x, int y, int width, int height) {
    HWND box = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Buscando dispositivos...",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
        x, y, width, height, parent, reinterpret_cast<HMENU>(id), g_instance, nullptr);
    SendMessageW(box, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    return box;
}

void CreateControls(HWND window) {
    CreateLabel(window, L"LEMON ASIO", 34, 22, 420, 42, g_titleFont);
    CreateLabel(window, L"USB AUDIO BRIDGE  /  WINDOWS 11  /  BUILD 0.1.1", 36, 64, 620, 22);

    HWND refresh = CreateWindowExW(0, L"BUTTON", L"Actualizar dispositivos",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        676, 45, 190, 34, window, reinterpret_cast<HMENU>(ID_REFRESH),
        g_instance, nullptr);
    SendMessageW(refresh, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);

    CreateLabel(window, L"ENTRADA  /  MICRÓFONO USB", 36, 112, 390, 22);
    g_inputCombo = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        36, 140, 390, 260, window, reinterpret_cast<HMENU>(ID_INPUT_COMBO),
        g_instance, nullptr);
    SendMessageW(g_inputCombo, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);

    CreateLabel(window, L"SALIDA  /  AURICULARES USB", 476, 112, 390, 22);
    g_outputCombo = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        476, 140, 390, 260, window, reinterpret_cast<HMENU>(ID_OUTPUT_COMBO),
        g_instance, nullptr);
    SendMessageW(g_outputCombo, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);

    CreateLabel(window, L"INFORMACIÓN DEL MICRÓFONO", 36, 190, 390, 22);
    g_inputDetails = CreateDetailsBox(window, ID_INPUT_DETAILS, 36, 218, 390, 142);

    CreateLabel(window, L"INFORMACIÓN DE LOS AURICULARES", 476, 190, 390, 22);
    g_outputDetails = CreateDetailsBox(window, ID_OUTPUT_DETAILS, 476, 218, 390, 142);

    CreateLabel(window, L"AJUSTES OBJETIVO PARA EL MOTOR  /  TODAVÍA NO APLICADOS",
                36, 400, 720, 22);
    CreateLabel(window, L"BUFFER", 36, 438, 100, 20);
    g_bufferCombo = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
        36, 464, 170, 220, window, reinterpret_cast<HMENU>(ID_BUFFER_COMBO),
        g_instance, nullptr);
    SendMessageW(g_bufferCombo, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    AddComboItems(g_bufferCombo, {L"64 samples", L"128 samples", L"256 samples",
                    L"512 samples", L"1024 samples"}, 2);

    CreateLabel(window, L"FRECUENCIA", 232, 438, 150, 20);
    g_rateCombo = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
        232, 464, 170, 220, window, reinterpret_cast<HMENU>(ID_RATE_COMBO),
        g_instance, nullptr);
    SendMessageW(g_rateCombo, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
    AddComboItems(g_rateCombo, {L"44100 Hz", L"48000 Hz", L"96000 Hz"}, 1);

    CreateLabel(window,
        L"La lista solo detecta dispositivos. Aún no transmite audio ni cambia la latencia.",
        36, 510, 820, 28);

    g_status = CreateWindowExW(0, L"STATIC", L"Inicializando...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        36, 568, 820, 26, window, reinterpret_cast<HMENU>(ID_STATUS),
        g_instance, nullptr);
    SendMessageW(g_status, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls(window);
        RefreshDevices();
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_REFRESH && HIWORD(wParam) == BN_CLICKED) {
            RefreshDevices();
            return 0;
        }
        if (LOWORD(wParam) == ID_INPUT_COMBO && HIWORD(wParam) == CBN_SELCHANGE) {
            UpdateDeviceDetails(true);
            return 0;
        }
        if (LOWORD(wParam) == ID_OUTPUT_COMBO && HIWORD(wParam) == CBN_SELCHANGE) {
            UpdateDeviceDetails(false);
            return 0;
        }
        break;

    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetTextColor(dc, RGB(226, 230, 234));
        SetBkMode(dc, TRANSPARENT);
        return reinterpret_cast<LRESULT>(g_backgroundBrush);
    }

    case WM_CTLCOLOREDIT: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetTextColor(dc, RGB(226, 230, 234));
        SetBkColor(dc, RGB(29, 32, 36));
        return reinterpret_cast<LRESULT>(g_panelBrush);
    }

    case WM_ERASEBKGND: {
        RECT rect;
        GetClientRect(window, &rect);
        FillRect(reinterpret_cast<HDC>(wParam), &rect, g_backgroundBrush);
        return 1;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    g_instance = instance;
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);

    g_font = CreateFontW(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_titleFont = CreateFontW(34, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_backgroundBrush = CreateSolidBrush(RGB(18, 20, 23));
    g_panelBrush = CreateSolidBrush(RGB(29, 32, 36));

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hbrBackground = g_backgroundBrush;
    wc.lpszClassName = L"LEMON_ASIO_MAIN_WINDOW";

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(nullptr, L"No se pudo registrar la ventana de LEMON ASIO.",
                    L"LEMON ASIO", MB_ICONERROR | MB_OK);
        return 1;
    }

    HWND window = CreateWindowExW(0, wc.lpszClassName,
        L"LEMON ASIO | USB Audio Bridge",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 910, 650,
        nullptr, nullptr, instance, nullptr);

    if (!window) {
        MessageBoxW(nullptr, L"No se pudo crear la ventana de LEMON ASIO.",
                    L"LEMON ASIO", MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    if (g_font) DeleteObject(g_font);
    if (g_titleFont) DeleteObject(g_titleFont);
    if (g_backgroundBrush) DeleteObject(g_backgroundBrush);
    if (g_panelBrush) DeleteObject(g_panelBrush);

    if (SUCCEEDED(comResult)) CoUninitialize();
    return static_cast<int>(message.wParam);
}
