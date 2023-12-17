@echo off

pushd \

for /F "tokens=* USEBACKQ" %%F in (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -prerelease -latest -products * -property installationPath`) do (
	set vs2019_install=%%F
)

for /F "tokens=* USEBACKQ" %%F in (`type "%vs2019_install%\VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt"`) do (
    set msvc_defaultver=%%F
)
echo Default MSVC version is %msvc_defaultver%
set msvc_path=%vs2019_install%\VC\Tools\MSVC\%msvc_defaultver%
c:
pushd "%vs2019_install%\VC\Auxiliary\Build"
call vcvarsall.bat x64
popd
%~d0
popd

