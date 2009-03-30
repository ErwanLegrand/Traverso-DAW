[Setup]
AppName=Traverso
AppVerName=Traverso 0.49.1
DefaultDirName={pf}\Traverso
DefaultGroupName=Traverso
PrivilegesRequired=none
UninstallDisplayIcon={app}\traverso.exe

[Files]
Source: "bin\traverso.exe"; DestDir: "{app}"
Source: "bin\cdrdao.exe"; DestDir: "{app}"
Source: "bin\wget.exe"; DestDir: "{tmp}"
Source: "bin\mingwm10.dll"; DestDir: "{app}"
Source: "bin\cygwin1.dll"; DestDir: "{app}"
Source: "bin\portaudio.dll"; DestDir: "{app}"
Source: "bin\libfftw3-3.dll"; DestDir: "{app}"
Source: "bin\libsndfile-1.dll"; DestDir: "{app}"
Source: "bin\libFLAC.dll"; DestDir: "{app}"
Source: "bin\libMAD.dll"; DestDir: "{app}"
Source: "bin\ogg.dll"; DestDir: "{app}"
Source: "bin\vorbis.dll"; DestDir: "{app}"
Source: "bin\vorbisfile.dll"; DestDir: "{app}"
Source: "bin\vorbisenc.dll"; DestDir: "{app}"
Source: "bin\wavpackdll.dll"; DestDir: "{app}"
Source: "bin\libsamplerate.dll"; DestDir: "{app}"
Source: "bin\QtCore4.dll"; DestDir: "{app}"
Source: "bin\QtGui4.dll"; DestDir: "{app}"
Source: "bin\QtXml4.dll"; DestDir: "{app}"


[Tasks]
Name: aspi; Description: "Download and Install ASPI drivers (needed for CD Burning)"

[Icons]
Name: "{group}\Traverso"; Filename: "{app}\traverso.exe"
Name: "{group}\Uninstall Traverso"; Filename: "{uninstallexe}"

[Run]
Filename: "{tmp}\wget.exe"; Parameters: "-O {tmp}/aspi.exe http://download.adaptec.com/software_pc/aspi/aspi_471a2.exe"; StatusMsg: "Downloading ASPI Drivers..."; Tasks: aspi
Filename: "{tmp}\aspi.exe"; StatusMsg: "Installing ASPI Driver Installer..."; Tasks: aspi
Filename: "C:\adaptec\aspi\aspiinst.exe"; StatusMsg: "Installing ASPI Drivers..."; Tasks: aspi

