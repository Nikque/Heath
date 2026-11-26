@echo off
rem Build the Windows release binaries of src\NMakefile with PGO, one after the other:
rem   wEdax-x86-64-v4.exe, wEdax-x86-64-v3.exe, wEdax-x86-64.exe, wEdax-x86-sse.exe, wEdax-x86.exe (vc-pgo-*),
rem   libedax-x64-v4.dll, libedax-x64-v3.dll, libedax-x64.dll (vc-pgo-lib-*).
rem usage: build-release.cmd <checkout> <log directory> <slots: 64 or 48> <eval.dat>
rem   <checkout>: a working tree whose bin\ may be overwritten, with problem\fforum-20-39.obf (the profile runs).
rem   <eval.dat>: the eval.dat of the release of these slots; the profile runs read it (PGO_EVAL).
rem Each program is linked again with /LTCG:PGO and recorded in <checkout>\bin\pgo-build.txt (src\pgo-record.cmd);
rem the record of an earlier run is renamed pgo-build.txt.old first. package-release.py takes the programs and the record.
rem The x86 targets are built from a native x86 environment: the instrumented program needs the x86 pgort.
rem Run it on one logical CPU (the processes inherit the affinity of this one): the profile runs use one thread.
setlocal EnableDelayedExpansion
set "ROOT=%~f1"
set "LOGS=%~f2"
set "SLOTS=%~3"
set "EVAL=%~f4"
if "%~4"=="" (
    echo usage: build-release.cmd ^<checkout^> ^<log directory^> ^<64^|48^> ^<eval.dat^>
    exit /b 2
)
if not "%SLOTS%"=="64" if not "%SLOTS%"=="48" (
    echo the slots are 64 or 48, not %SLOTS%
    exit /b 2
)
if not exist "%ROOT%\problem\fforum-20-39.obf" (
    echo %ROOT%\problem\fforum-20-39.obf is not found.
    exit /b 2
)
if not exist "%EVAL%" (
    echo %EVAL% is not found.
    exit /b 2
)
if not exist "%LOGS%" mkdir "%LOGS%"
if exist "%ROOT%\bin\pgo-build.txt" move /Y "%ROOT%\bin\pgo-build.txt" "%ROOT%\bin\pgo-build.txt.old" >nul
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
set VSDEV="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
set FAILED=0
for %%t in (vc-pgo-x64-v4:x64 vc-pgo-x64-v3:x64 vc-pgo-x64:x64 vc-pgo-x86-sse:x86 vc-pgo-x86:x86 vc-pgo-lib-x64-v4:x64 vc-pgo-lib-x64-v3:x64 vc-pgo-lib-x64:x64) do (
    for /f "tokens=1,2 delims=:" %%a in ("%%t") do (
        echo %%a start !time!
        cmd /c "%VSDEV% -arch=%%b -host_arch=%%b >nul && cd /d "%ROOT%\src" && nmake /nologo -f NMakefile EVAL_SLOTS=%SLOTS% "PGO_EVAL=%EVAL%" %%a > "%LOGS%\%%a.log" 2>&1"
        set CODE=!errorlevel!
        if not "!CODE!"=="0" set FAILED=1
        echo %%a exit !CODE! !time!
    )
)
if exist "%ROOT%\bin\pgo-build.txt" type "%ROOT%\bin\pgo-build.txt"
echo all done
exit /b %FAILED%
