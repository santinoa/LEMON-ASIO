# LEMON ASIO — USB Audio Bridge (Milestone 0.1.1)

LEMON ASIO is a Windows 11 audio-driver project. This milestone is a **device-discovery prototype**, updated for the intended setup: a USB microphone as input and separate USB headphones as output.

## What this build does

- Lists active Windows audio capture endpoints (USB microphones and other inputs).
- Lists active Windows audio render endpoints (USB headphones, speakers and other outputs).
- Marks the default Windows input and output independently.
- Lets you inspect each endpoint's current shared-mode mix format and reported device periods.
- Keeps the input and output selections separate, matching the intended USB microphone + USB headphones setup.

## What it does not do yet

- It does not stream audio or process FL Studio audio.
- It does not install/register an ASIO driver.
- It does not change Windows sound settings, sample rate, buffer size or latency.
- It does not guarantee low latency or synchronize independent USB device clocks yet.

## Requirements

- Windows 10/11 x64 (target platform: Windows 11 x64)
- Visual Studio 2022 Community or Build Tools with **Desktop development with C++**
- MSVC v143 toolset
- Windows 10/11 SDK
- CMake 3.21 or newer

## Build

Open **x64 Native Tools Command Prompt for VS 2022**, move to this folder, then run:

```bat
cmake -S . -B build -A x64
cmake --build build --config Release
```

The executable will be created at:

```text
build\Release\LEMON_ASIO.exe
```

Alternatively, open this folder in Visual Studio 2022 using **File > Open > Folder** and select the CMake project.

## First test

1. Connect the USB headphones and the USB microphone.
2. In Windows Settings > System > Sound, verify that the headphones appear under Output and the microphone under Input.
3. Run `LEMON_ASIO.exe` and press **Actualizar dispositivos**.
4. Check the two lists separately. The microphone should appear under **Entrada** and the headphones under **Salida**.
5. If one is missing, check Windows sound settings, USB connection and microphone privacy permissions.

## Use the setup in FL Studio now

The current FL Studio installation includes **FL Studio ASIO**, which can select a separate USB input and output. In FL Studio, press F10 > Audio, select **FL Studio ASIO**, open its panel, choose the USB microphone in Input and the USB headphones in Output. Then select the microphone as the input on an Insert mixer track. If FL Studio ASIO does not expose one of the devices, try ASIO4ALL and activate the microphone's input and headphones' output in its advanced device list.

This is the fastest way to achieve the required setup while LEMON ASIO is being developed. See Image-Line's official USB microphone guide:
https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/recording_audio_USBmic.htm

## Architecture roadmap

1. **0.1.1 — Separate endpoint discovery:** implemented with Windows MMDevice/WASAPI metadata APIs.
2. **0.2 — Duplex WASAPI engine:** implement real capture from the USB microphone and playback to USB headphones, initially in shared mode, with event-driven buffering, device-loss recovery and clock-drift handling/resampling.
3. **0.3 — ASIO interface:** implement the ASIO driver contract and host callbacks so an ASIO host such as FL Studio can enumerate the driver.
4. **0.4 — Control panel + installer:** persist supported settings, register the 64-bit driver correctly, and add safe install/uninstall routines.
5. **0.5 — Validation:** test recording, playback, sample-rate negotiation, buffer stability, device switching and measured latency.

## ASIO SDK licensing note

No Steinberg ASIO SDK files are included in this repository. Before integrating the SDK, choose and comply with the licensing route Steinberg currently offers. Obtain any SDK materials from Steinberg's official developer portal rather than from an unverified package.

## Project source license

The original prototype source in this repository is provided under the MIT License (see `LICENSE.txt`). This does not grant rights to third-party SDK materials or trademarks.

## Compile without installing Visual Studio on your PC

This repository includes `.github/workflows/build-windows.yml`. Push the source to a GitHub **repository** (not a Gist), open the repository's **Actions** tab, select **Build LEMON ASIO for Windows**, and choose **Run workflow**. GitHub's hosted Windows runner performs the MSVC/CMake build. When it finishes, open that workflow run and download the artifact named `LEMON-ASIO-Windows-x64`.

No local Visual Studio installation or C++ compiler is needed for this cloud build. A GitHub account and a repository with Actions enabled are required. This workflow builds only the current device-discovery prototype; it does not create or register an ASIO driver.
