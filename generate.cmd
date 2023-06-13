set CURRENT_DIR=%CD%
tools\cmake\win32\bin\cmake.exe -S "%CURRENT_DIR%" -B "%CURRENT_DIR%\build" -G "Visual Studio 17 2022" -A x64 --toolchain "%CURRENT_DIR%\tools\cmake\toolchains\msvc-win_x64.cmake"
start tools\cmake\win32\bin\cmake-gui.exe -S "%CURRENT_DIR%" -B "%CURRENT_DIR%\build"