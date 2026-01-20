@echo off
setlocal

set DLL=redistributable_bin/win64/steam_api64.dll
set DEF=steam_api64.def
set LIB=redistributable_bin/win64/libsteam_api64.a

if not exist %DLL% (
    echo ERROR: %DLL% not found
    exit /b 1
)

echo Generating DEF file from %DLL%...
gendef %DLL%
if errorlevel 1 (
    echo ERROR: gendef failed
    exit /b 1
)

echo Creating MinGW import library %LIB%...
dlltool -d %DEF% -l %LIB% -k
if errorlevel 1 (
    echo ERROR: dlltool failed
    exit /b 1
)

echo.
echo SUCCESS!
echo Generated: %LIB%
echo.

endlocal
pause
