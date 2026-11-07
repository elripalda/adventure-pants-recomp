@echo off
title Adventure Pants Recomp - Package release
cd /d "%~dp0"
call tools\env.bat || (pause & exit /b 1)
set "VERSION=1.0.0"
set /p VERSION=Version number [1.0.0]: 
set "NAME=AdventurePantsRecomp-v%VERSION%-Windows-x64"
set "STAGE=%ROOT%\release\%NAME%"

if not exist "%OUT%\AdventurePantsRecomp.exe" (echo Build it first with 3_build.bat & pause & exit /b 1)
rmdir /s /q "%STAGE%" 2>nul
mkdir "%STAGE%"
rem Just the program: no game files, no settings file (the exe and RecompLauncher make their own).
copy /y "%OUT%\AdventurePantsRecomp.exe" "%STAGE%\" >nul
for %%f in ("%OUT%\*.dll") do copy /y "%%f" "%STAGE%\" >nul
copy /y "%PROJ%\recomplauncher.json" "%STAGE%\" >nul
copy /y "%ROOT%\README.md" "%STAGE%\" >nul 2>nul

del /q "%ROOT%\release\%NAME%.zip" 2>nul
powershell -NoProfile -Command "Compress-Archive -Path '%STAGE%\*' -DestinationPath '%ROOT%\release\%NAME%.zip'"
del /q "%ROOT%\release\AdventurePantsRecomp-source.zip" 2>nul
powershell -NoProfile -Command "$items = @('%PROJ%\src','%PROJ%\res','%PROJ%\CMakeLists.txt','%PROJ%\CMakePresets.json','%PROJ%\FancyPants_manifest.toml','%PROJ%\fancypants_functions.toml','%PROJ%\recomplauncher.json','%ROOT%\tools'); Compress-Archive -Path $items -DestinationPath '%ROOT%\release\AdventurePantsRecomp-v%VERSION%-source.zip' -Force"

echo.
echo  Made release\%NAME%.zip and the source zip next to it.
echo.
pause
