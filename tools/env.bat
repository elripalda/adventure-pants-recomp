@echo off
rem Shared settings for the build scripts. Edit the two paths below if you move things.
set "SDK=%USERPROFILE%\Desktop\rexglue-sdk-0.10.1-win-amd64\win-amd64"
set "PKG=%USERPROFILE%\Desktop\Fancy Pants Adventures, The (World) (XBLA)\58410A9F\000D0000\6BD7BB6917CE7BC0B816C280ACFD9BD78471228258"

set "ROOT=%~dp0.."
for %%i in ("%ROOT%") do set "ROOT=%%~fi"
set "PROJ=%ROOT%\project"
set "OUT=%ROOT%\AdventurePantsRecomp"
set "LOGS=%ROOT%\logs"
if not exist "%LOGS%" mkdir "%LOGS%"

rem One-time move from the old FancyPants folder name.
if exist "%ROOT%\FancyPants" if not exist "%OUT%" ren "%ROOT%\FancyPants" AdventurePantsRecomp
if exist "%OUT%\FancyPants.toml" if not exist "%OUT%\AdventurePantsRecomp.toml" ren "%OUT%\FancyPants.toml" AdventurePantsRecomp.toml
if exist "%OUT%\FancyPants.exe" del /q "%OUT%\FancyPants.exe"

rem The build reads its own copy of default.xex.
if not exist "%PROJ%\input\default.xex" if exist "%OUT%\game\default.xex" (
  mkdir "%PROJ%\input" 2>nul
  copy /y "%OUT%\game\default.xex" "%PROJ%\input\default.xex" >nul
)

set "PATH=C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;%LOCALAPPDATA%\Microsoft\WinGet\Links;%PATH%"

if not exist "%SDK%\bin\rexglue.exe" (
  echo Could not find the ReXGlue SDK at:
  echo   "%SDK%"
  echo Put the rexglue-sdk-0.10.1-win-amd64 folder back on your Desktop, or edit tools\env.bat.
  exit /b 1
)
exit /b 0
