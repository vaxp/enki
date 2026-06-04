@echo off
setlocal
set "CLI_EXE=%~dp0build-Win\tools\cli\enki.exe"
if exist "%CLI_EXE%" (
    "%CLI_EXE%" %*
) else (
    echo [ENKI] CLI not built yet. Run 'ninja -C build-Win tools/cli/enki.exe' first.
    exit /b 1
)
endlocal
