# LEMON Audio — experimental ASIO-compatible USB audio driver

**Experimental Windows x64 build.** This branch is the first integration milestone; it is not yet declared stable or verified on every PC.

LEMON Audio is being built for a practical setup: use one USB device as the recording input (for example, a USB microphone) and a different USB device as the playback output (for example, USB headphones), from a DAW such as FL Studio.

## What this branch integrates

- KoordASIO 2.3.1 as a pinned third-party codebase, itself derived from FlexASIO.
- Separate WASAPI input and output endpoint selection through the ASIO driver interface.
- Shared mode as the safe default, with Exclusive mode available in the control panel.
- Buffer controls from 32 to 2048 samples.
- A branded LEMON Audio control panel and Windows x64 installer.
- A unique COM class ID and separate registry/configuration namespace so this driver does not overwrite the upstream KoordASIO registration.

The upstream code is referenced as a Git submodule at a fixed commit. The build workflow applies a small, auditable branding/licensing script in the GitHub Actions workspace; it does not rewrite the upstream project history.

## Build without Visual Studio on your PC

1. Open the Actions tab of this repository.
2. Choose **Build LEMON Audio (experimental)**.
3. Select **Run workflow**.
4. When the run succeeds, download the artifact **LEMON-Audio-0.3.0-Windows-x64**.

GitHub Actions uses a hosted Windows runner with MSVC, CMake, Ninja, Qt and Inno Setup. Your own computer only needs to download and run the installer.

## First test in FL Studio

1. Install the experimental package.
2. Open FL Studio and go to **Options → Audio Settings**.
3. Select **LEMON Audio** in the device list.
4. Open its ASIO panel and choose the USB microphone under Input and USB headphones under Output.
5. Start with Shared mode and a 64-sample buffer for lower delay. If you hear clicks, pops, or dropouts, raise it to 128; if 64 is stable, test 32. Keep Exclusive mode as an optional experiment rather than the default.

The actual latency and stability depend on the two devices and their clocks. Start with a stable setting first; do not assume a smaller buffer will always work better.

## Important status

- The older root-level 0.1.x application remains a device-enumeration prototype; it is not the ASIO driver.
- The 0.2.0 WASAPI monitor prototype is separate and is not used by this driver.
- This branch has not yet been validated on your exact USB microphone/headphone pair or in your installed FL Studio. A green CI build only confirms compilation and packaging, not real-world audio quality.
- Upgrades preserve an existing user's saved buffer choice. On an existing installation, open LEMON Audio Control and choose 64 samples manually; fresh installs default to 64.
- Do not install it on top of KoordASIO if you want to compare the two. The COM registration and configuration paths are separated, but test one driver at a time.

## Licensing

The integrated driver uses the Steinberg ASIO SDK 2.3.4 under its GPLv3 option. Accordingly, the combined LEMON Audio driver distribution in this branch is GPL-3.0-only. The original device-enumeration prototype's MIT notice is preserved separately in LICENSE-ORIGINAL-PROTOTYPE.txt. See THIRD_PARTY_NOTICES.md and the pinned upstream submodule for component-level notices.
