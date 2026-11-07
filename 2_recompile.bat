@echo off
title Adventure Pants Recomp - Recompile
cd /d "%~dp0"
call tools\env.bat || (pause & exit /b 1)
echo.
echo  Unpacking your copy and recompiling the game code
echo  -------------------------------------------------
echo.

set PY=py -3
where py >nul 2>&1 || set PY=python

if not exist "%OUT%\game\default.xex" (
  echo [1/2] Unpacking your Xbox Live Arcade package...
  if not exist "%PKG%" (echo Could not find your game package at: & echo   "%PKG%" & echo Edit the PKG line in tools\env.bat. & pause & exit /b 1)
  %PY% -I tools\stfs_extract.py "%PKG%" "%OUT%\game" || (echo Unpacking failed. & pause & exit /b 1)
) else (
  echo [1/2] Game files already unpacked.
)
if not exist "%PROJ%\input" mkdir "%PROJ%\input"
copy /y "%OUT%\game\default.xex" "%PROJ%\input\default.xex" >nul

echo [2/2] Recompiling the Xbox 360 code to C++ - this can take a few minutes...
pushd "%PROJ%"
del /q "generated\default\codegen.stamp" "generated\default\codegen.build.stamp" 2>nul
"%SDK%\bin\rexglue.exe" codegen FancyPants_manifest.toml --ignore-stamp --log-file "%LOGS%\2_codegen.log"
set ERR=%ERRORLEVEL%
popd
if not "%ERR%"=="0" (echo Recompile failed - see logs\2_codegen.log & pause & exit /b 1)

echo.
echo  Done. Next: double-click 3_build.bat
echo.
pause
