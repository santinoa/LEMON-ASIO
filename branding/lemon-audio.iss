#ifndef ApplicationVersion
  #define ApplicationVersion "0.3.5"
#endif

[Setup]
AppId={{7B6B5EA7-1A41-4AB9-8D8D-36D084D1A0AC}
AppName=LEMON Audio
AppVersion={#ApplicationVersion}
AppVerName=LEMON Audio {#ApplicationVersion}
AppPublisher=LEMON Audio contributors
DefaultDirName={autopf}\LEMON Audio
DefaultGroupName=LEMON Audio
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
WizardStyle=modern
OutputDir=..\Output
OutputBaseFilename=LEMON-Audio-Setup-{#ApplicationVersion}
Compression=lzma2
SolidCompression=yes
UninstallDisplayName=LEMON Audio
Uninstallable=yes

[Files]
Source: "..\deploy\LEMONAudio.dll"; DestDir: "{app}"; Flags: ignoreversion regserver 64bit
Source: "..\deploy\*"; DestDir: "{app}"; Excludes: "LEMONAudio.dll"; Flags: ignoreversion recursesubdirs 64bit
Source: "..\LICENSE.txt"; DestDir: "{app}\licenses"; Flags: ignoreversion
Source: "..\GPL-3.0.txt"; DestDir: "{app}\licenses"; Flags: ignoreversion
Source: "..\THIRD_PARTY_NOTICES.md"; DestDir: "{app}\licenses"; Flags: ignoreversion

[Icons]
Name: "{group}\LEMON Audio Control"; Filename: "{app}\LEMONAudioControl.exe"
Name: "{autodesktop}\LEMON Audio Control"; Filename: "{app}\LEMONAudioControl.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"

[Registry]
Root: HKLM64; Subkey: "Software\LEMONAudio"; Flags: uninsdeletekeyifempty
Root: HKLM64; Subkey: "Software\LEMONAudio\Driver"; Flags: uninsdeletekeyifempty
Root: HKLM64; Subkey: "Software\LEMONAudio\Driver\Install"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"; Flags: uninsdeletekey

[Run]
Filename: "{app}\LEMONAudioControl.exe"; Parameters: "-defaults"; Flags: runhidden waituntilterminated
Filename: "{app}\LEMONAudioControl.exe"; Description: "Open LEMON Audio Control"; Flags: postinstall nowait skipifsilent
