[Setup]
AppName=Traverso
AppVerName=Traverso 0.42.0-rc1
DefaultDirName={pf}\Traverso
DefaultGroupName=Traverso
PrivilegesRequired=none
UninstallDisplayIcon={app}\traverso.exe

[Files]
Source: "traverso.exe"; DestDir: "{app}"
Source: "3thparty\bin\cdrdao.exe"; DestDir: "{app}"
Source: "3thparty\bin\wget.exe"; DestDir: "{tmp}"
Source: "3thparty\lib\cygwin1.dll"; DestDir: "{app}"
Source: "3thparty\lib\libfftw3-3.dll"; DestDir: "{app}"
Source: "3thparty\lib\libsndfile-1.dll"; DestDir: "{app}"
Source: "3thparty\lib\libFLAC.dll"; DestDir: "{app}"
Source: "3thparty\lib\libMAD.dll"; DestDir: "{app}"
Source: "3thparty\lib\ogg.dll"; DestDir: "{app}"
Source: "3thparty\lib\vorbis.dll"; DestDir: "{app}"
Source: "3thparty\lib\vorbisfile.dll"; DestDir: "{app}"
Source: "3thparty\lib\vorbisenc.dll"; DestDir: "{app}"
Source: "3thparty\lib\wavpackdll.dll"; DestDir: "{app}"
Source: "3thparty\lib\libsamplerate.dll"; DestDir: "{app}"
Source: "mingwm10.dll"; DestDir: "{app}"
Source: "QtCore4.dll"; DestDir: "{app}"
Source: "QtGui4.dll"; DestDir: "{app}"
Source: "QtOpenGL4.dll"; DestDir: "{app}"
Source: "QtXml4.dll"; DestDir: "{app}"


[Tasks]
Name: aspi; Description: "Download and Install ASPI drivers (needed for CD Burning)"

[Icons]
Name: "{group}\Traverso"; Filename: "{app}\traverso.exe"
Name: "{group}\Uninstall Traverso"; Filename: "{uninstallexe}"

[Run]
Filename: "{tmp}\wget.exe"; Parameters: "-O {tmp}/aspi.exe http://download.adaptec.com/software_pc/aspi/aspi_471a2.exe"; StatusMsg: "Downloading ASPI Drivers..."; Tasks: aspi
Filename: "{tmp}\aspi.exe"; StatusMsg: "Installing ASPI Driver Installer..."; Tasks: aspi
Filename: "C:\adaptec\aspi\aspiinst.exe"; StatusMsg: "Installing ASPI Drivers..."; Tasks: aspi

