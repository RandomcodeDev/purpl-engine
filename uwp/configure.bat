@echo off

set "generator=Visual Studio 17 2022"
set windowsversion=%1

if "%windowsversion%" == "" echo You need to specify the Windows SDK version to use as the third argument to build.bat & exit /b

cmake -S. -Bbuild-uwp -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=%windowsversion% -G"%generator%" -DPurplHostTools_DIR=%~dp0..\..\build-tools-uwp

