#!/usr/bin/env python3
"""Apply the LEMON Audio branding and GPLv3 ASIO SDK selection to pinned KoordASIO 2.3.1 sources.

This script runs only in the GitHub Actions build workspace. It does not modify
the upstream git submodule commit or the repository's main branch.
"""
from pathlib import Path
import re
import sys

UPSTREAM = Path(sys.argv[1] if len(sys.argv) > 1 else "third_party/KoordASIO")

# Fresh identifiers: a separate COM class and type library, so installing
# LEMON Audio never overwrites an existing KoordASIO registration.
TYPELIB_GUID = "0277E5A6-86E7-40E7-9402-92601CD2D80F"
DRIVER_GUID = "2B8A995A-7481-4D4B-9B1C-296070F20A75"
OLD_DRIVER_GUID = "EE8F66BC-9646-4487-ACB9-0F1C7D68D1FA"
OLD_TYPELIB_GUID = "42DCBA0A-BA58-4BA7-918F-D34A121F9E8D"


def file_path(rel: str) -> Path:
    path = UPSTREAM / rel
    if not path.is_file():
        raise FileNotFoundError(f"Expected upstream file is missing: {path}")
    return path


def read(rel: str) -> str:
    return file_path(rel).read_text(encoding="utf-8")


def write(rel: str, content: str) -> None:
    file_path(rel).write_text(content, encoding="utf-8", newline="\n")


def replace_exact(rel: str, old: str, new: str, expected: int = 1) -> None:
    content = read(rel)
    count = content.count(old)
    if count != expected:
        raise RuntimeError(
            f"{rel}: expected {expected} occurrence(s), found {count} for {old!r}"
        )
    write(rel, content.replace(old, new))


def replace_all(rel: str, old: str, new: str, minimum: int = 1) -> None:
    content = read(rel)
    count = content.count(old)
    if count < minimum:
        raise RuntimeError(
            f"{rel}: expected at least {minimum} occurrence(s), found {count} for {old!r}"
        )
    write(rel, content.replace(old, new))


def replace_regex_once(rel: str, pattern: str, replacement: str) -> None:
    content = read(rel)
    updated, count = re.subn(pattern, replacement, content, count=1, flags=re.MULTILINE | re.DOTALL)
    if count != 1:
        raise RuntimeError(f"{rel}: expected one regex match, found {count}: {pattern}")
    write(rel, updated)


# Refuse to brand an unexpected upstream revision.
replace_exact(
    "CMakeLists.txt",
    "project(KoordASIO VERSION 2.3.1 LANGUAGES NONE)",
    "project(LEMONAudio VERSION 0.3.0 LANGUAGES NONE)",
)

# Use Steinberg's first GPLv3-open-source SDK release instead of the old
# pre-open-source 2.3.3 package pinned by the upstream ASIOUtil submodule.
asio_util = "src/dechamps_ASIOUtil/CMakeLists.txt"
replace_exact(
    asio_util,
    "https://download.steinberg.net/sdk_downloads/asiosdk_2.3.3_2019-06-14.zip",
    "https://download.steinberg.net/sdk_downloads/ASIO-SDK_2.3.4_2025-10-15.zip",
)
asio_cmake = read(asio_util)
asio_cmake, hash_count = re.subn(r"^\s*URL_HASH SHA1=[^\r\n]*\r?\n", "", asio_cmake, count=1, flags=re.MULTILINE)
if hash_count != 1:
    raise RuntimeError("ASIOUtil CMake: expected the pinned 2.3.3 SDK hash to remove")
asio_cmake = asio_cmake.replace(
    'URL "https://download.steinberg.net/sdk_downloads/ASIO-SDK_2.3.4_2025-10-15.zip"\n\tUSES_TERMINAL_DOWNLOAD TRUE',
    'URL "https://download.steinberg.net/sdk_downloads/ASIO-SDK_2.3.4_2025-10-15.zip"\n\tURL_HASH SHA256=d5ebf0c20dd2c5f43771fd0c1418f4b361bf52434ee670097cfa6b3a335e2eca\n\tUSES_TERMINAL_DOWNLOAD TRUE',
)
write(asio_util, asio_cmake)

# Rename the built COM DLL target while keeping upstream internal C++ symbols.
driver_cmake = "src/flexasio/FlexASIO/CMakeLists.txt"
replace_all(driver_cmake, "KoordASIO", "LEMONAudio", minimum=3)
replace_exact(
    driver_cmake,
    'PROJECT_DESCRIPTION="LEMONAudio ASIO Driver DLL"',
    'PROJECT_DESCRIPTION="LEMON Audio ASIO-compatible Driver DLL"',
)

# The self-test target must link to the renamed DLL target as well.
test_cmake = "src/flexasio/FlexASIOTest/CMakeLists.txt"
replace_exact(test_cmake, "PRIVATE KoordASIO", "PRIVATE LEMONAudio")

# The ASIO registry name, COM ProgID and class identifier must all be unique.
rgs = "src/flexasio/FlexASIO/flexasio.rgs"
replace_exact(rgs, "koordasio.KoordASIO.1", "lemonaudio.LEMONAudio.1", expected=2)
replace_all(rgs, "KoordASIO", "LEMON Audio", minimum=2)
replace_all(rgs, OLD_DRIVER_GUID, DRIVER_GUID, minimum=2)

idl = "src/flexasio/FlexASIO/flexasio.idl"
replace_exact(idl, OLD_TYPELIB_GUID, TYPELIB_GUID)
replace_exact(idl, OLD_DRIVER_GUID, DRIVER_GUID)
replace_exact(idl, "library LFlexASIO", "library LLEMONAudio")

# Host-visible driver name.
replace_exact(
    "src/flexasio/FlexASIO/cflexasio.cpp",
    'strcpy_s(name, 32, "KoordASIO");',
    'strcpy_s(name, 32, "LEMON Audio");',
)
replace_all("src/flexasio/FlexASIO/flexasio.cpp", "KoordASIO", "LEMON Audio", minimum=4)

# Control-panel lookup, executable name and documentation destination.
control_panel = "src/flexasio/FlexASIO/control_panel.cpp"
replace_exact(
    control_panel,
    r'L"Software\\Koord\\KoordASIO\\Install"',
    r'L"Software\\LEMONAudio\\Driver\\Install"',
)
replace_all(control_panel, "KoordASIOControl.exe", "LEMONAudioControl.exe")
replace_exact(
    control_panel,
    r'std::wstring(L"https://github.com/kormix-io/KoordASIO/blob/") + ConvertFromUTF8(::dechamps_CMakeUtils_gitDescription)',
    r'std::wstring(L"https://github.com/santinoa/LEMON-ASIO/blob/dev/lemon-audio-asio")',
)
replace_all(control_panel, "KoordASIO", "LEMON Audio", minimum=2)

# Driver and GUI share one per-user configuration filename.
replace_exact(
    "src/flexasio/FlexASIO/config.cpp",
    'L".KoordASIO.toml"',
    'L".LEMONAudio.toml"',
)
log_cpp = "src/flexasio/FlexASIO/log.cpp"
replace_exact(log_cpp, '"KoordASIO.log"', '"LEMONAudio.log"')
replace_all(log_cpp, "KoordASIO ", "LEMON Audio ", minimum=1)

# File version metadata keeps upstream copyright attribution.
replace_exact(
    "src/flexasio/versioninfo.rc",
    'VALUE "ProductName", "FlexASIO https://github.com/dechamps/FlexASIO"',
    'VALUE "ProductName", "LEMON Audio (derived from FlexASIO and KoordASIO)"',
)

# Control-panel target name and version macro.
gui_cmake = "src/kdasioconfig/CMakeLists.txt"
replace_all(gui_cmake, "KOORDASIO_ROOT_CMAKE", "LEMON_AUDIO_ROOT_CMAKE", minimum=2)
replace_exact(
    gui_cmake,
    'project\\\\(KoordASIO VERSION ([0-9.]+)',
    'project\\\\(LEMONAudio VERSION ([0-9.]+)',
)
replace_all(gui_cmake, "KOORDASIO_VERSION", "LEMON_AUDIO_VERSION", minimum=2)
replace_exact(gui_cmake, 'OUTPUT_NAME "KoordASIOControl"', 'OUTPUT_NAME "LEMONAudioControl"')

main_cpp = "src/kdasioconfig/main.cpp"
replace_all(main_cpp, "KoordASIOControl.log", "LEMONAudioControl.log", minimum=1)
replace_all(main_cpp, "KoordASIOControl", "LEMONAudioControl", minimum=1)
replace_all(main_cpp, "KoordASIO", "LEMON Audio", minimum=1)

config_model = "src/kdasioconfig/configmodel.cpp"
replace_all(config_model, "KOORDASIO_VERSION", "LEMON_AUDIO_VERSION", minimum=2)
replace_exact(config_model, "/.KoordASIO.toml", "/.LEMONAudio.toml")
replace_exact(
    config_model,
    'QStringLiteral("kormix"), QStringLiteral("KoordASIO")',
    'QStringLiteral("LEMONAudio"), QStringLiteral("LEMONAudio")',
)
replace_exact(
    config_model,
    "https://github.com/kormix-io/KoordASIO/releases",
    "https://github.com/santinoa/LEMON-ASIO/releases",
)
replace_exact(
    config_model,
    "https://github.com/kormix-io/KoordASIO",
    "https://github.com/santinoa/LEMON-ASIO",
)
replace_exact(
    config_model,
    "https://kormix-io.github.io/KoordASIO/",
    "https://github.com/santinoa/LEMON-ASIO/blob/dev/lemon-audio-asio/CONFIGURATION.md",
)
replace_all(config_model, "KoordASIO", "LEMON Audio", minimum=0)

replace_exact(
    "src/kdasioconfig/appinstance.cpp",
    "KoordASIOControl-",
    "LEMONAudioControl-",
)
replace_all("src/kdasioconfig/traymanager.cpp", "KoordASIO", "LEMON Audio", minimum=2)

qml = "src/kdasioconfig/main.qml"
replace_exact(
    qml,
    'ToolTip.text: "kormix-io.github.io/KoordASIO"',
    'ToolTip.text: "LEMON Audio project on GitHub"',
)
replace_all(qml, "KoordASIO", "LEMON Audio", minimum=4)
replace_exact(qml, 'readonly property color clrBg: "#313539"', 'readonly property color clrBg: "#111315"')
replace_exact(qml, 'readonly property color clrSectionBorder: "#6a6e74"', 'readonly property color clrSectionBorder: "#3c4248"')

logo = r'''                    ColumnLayout {
                        spacing: 3
                        Label {
                            text: "LEMON AUDIO"
                            color: "#B6F36A"
                            font.bold: true
                            font.pixelSize: 18
                        }
                        Label {
                            text: "ASIO-COMPATIBLE USB AUDIO DRIVER"
                            color: clrMuted
                            font.pixelSize: 10
                            font.letterSpacing: 0.5
                            Layout.leftMargin: 2
                        }
                    }
'''
replace_regex_once(
    qml,
    r'^                    Image \{\n                        source: "qrc:/images/logo\.png".*?^                    \}\n',
    logo,
)

footer_brand = r'''                Label {
                    text: "LEMON AUDIO"
                    color: "#B6F36A"
                    font.bold: true
                    font.pixelSize: 10
                    Layout.alignment: Qt.AlignVCenter
                }
'''
replace_regex_once(
    qml,
    r'^                Image \{\n                    source: "qrc:/images/frame\.png".*?^                \}\n',
    footer_brand,
)

# Remove the two upstream brand graphics that the customized panel no longer uses.
qrc = "src/kdasioconfig/app.qrc"
replace_exact(qrc, '        <file alias="images/logo.png">images/KoordASIO Logo.png</file>\n', "")
replace_exact(qrc, '        <file alias="images/frame.png">images/Frame.png</file>\n', "")

print("LEMON Audio branding applied to KoordASIO 2.3.1.")
print(f"COM CLSID: {{{DRIVER_GUID}}}")
print(f"Type library ID: {{{TYPELIB_GUID}}}")
print("ASIO SDK: 2.3.4 (GPLv3 option)")
