@echo off
title Fancy Pants PC - Rebuild
cd /d "%~dp0"
call tools\env.bat || (pause & exit /b 1)
echo.
echo  Rebuilding after a fix (recompile + build)
echo  ------------------------------------------
echo.
pushd "%PROJ%"
del /q "%PROJ%\generated\default\codegen.stamp" "%PROJ%\generated\default\codegen.build.stamp" 2>nul
"%SDK%\bin\rexglue.exe" codegen FancyPants_manifest.toml --ignore-stamp --log-file "%LOGS%\2_codegen.log"
set ERR=%ERRORLEVEL%
popd
if not "%ERR%"=="0" (echo Recompile failed - send me logs\2_codegen.log & pause & exit /b 1)
call 3_build.bat
