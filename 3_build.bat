@echo off
title Adventure Pants Recomp - Build
cd /d "%~dp0"
call tools\env.bat || (pause & exit /b 1)
call tools\vsenv.bat || (pause & exit /b 1)
echo.
echo  Building AdventurePantsRecomp.exe
echo  ---------------------------------
echo.

pushd "%PROJ%"
cmake -S . -B out\build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=23 "-DCMAKE_C_FLAGS=-march=x86-64-v2" "-DCMAKE_CXX_FLAGS=-march=x86-64-v2" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_PREFIX_PATH="%SDK%" > "%LOGS%\3_configure.log" 2>&1
if errorlevel 1 (popd & echo Configure failed - send me logs\3_configure.log & pause & exit /b 1)
rem Remove the last exe first, so a failed build can never look like a good one.
for /r out\build %%f in (AdventurePantsRecomp.exe) do if exist "%%f" del /q "%%f"
cmake --build out\build 2>&1 | powershell -NoProfile -Command "$input | Tee-Object -FilePath '%LOGS%\3_build.log'"
set "EXE="
for /r out\build %%f in (AdventurePantsRecomp.exe) do if exist "%%f" set "EXE=%%f"
if not defined EXE (popd & echo Build failed - send me logs\3_build.log & pause & exit /b 1)
for %%d in ("%EXE%") do set "EXEDIR=%%~dpd"
popd

if not exist "%OUT%" mkdir "%OUT%"
copy /y "%EXE%" "%OUT%\" >nul 2>&1
if errorlevel 1 (
  echo.
  echo  The game is still open, so the new version couldn't be copied in.
  echo  Close Adventure Pants Recomp and run 3_build.bat again.
  echo.
  pause
  exit /b 1
)
for %%f in ("%EXEDIR%*.dll") do copy /y "%%f" "%OUT%\" >nul
for %%f in (rexruntime.dll rexgpu-xenos.dll TracyClient.dll) do copy /y "%SDK%\bin\%%f" "%OUT%\" >nul
copy /y "%PROJ%\recomplauncher.json" "%OUT%\" >nul
if not exist "%OUT%\AdventurePantsRecomp.toml" copy /y tools\AdventurePantsRecomp.toml "%OUT%\" >nul

echo.
echo  Done! Double-click AdventurePantsRecomp\AdventurePantsRecomp.exe to play.
echo  Settings are in AdventurePantsRecomp\AdventurePantsRecomp.toml
echo.
pause
