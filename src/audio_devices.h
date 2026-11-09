#pragma once

#include <string>
#include <vector>

struct AudioDeviceInfo {
    std::wstring id;
    std::wstring name;
    std::wstring details;
    bool isDefault = false;
};

// input=true enumerates capture endpoints (microphones); input=false enumerates render endpoints (headphones/speakers).
// This only discovers Windows audio endpoints and reads metadata; it does not stream audio.
std::vector<AudioDeviceInfo> EnumerateAudioDevices(bool input, std::wstring& errorMessage);
