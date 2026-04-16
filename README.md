```sh
meson setup build --prefix=/usr --buildtype=release
# or full build with webview enabled
meson setup build --prefix=/usr --buildtype=release -Denable_webview=true
meson compile -C build
meson install -C build
```

cmd.exe /c "call ""C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"" && C:\msys64\ucrt64\bin\meson.exe setup build-Win --backend ninja -Dbuildtype=release -Db_vscrt=mt"


cmd.exe /c "call ""C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"" && ninja -C build-Win"


$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH; ninja -C build-android 

$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH; python3 scripts/package_android_apk.py 2>&1 | Out-String

$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH; python3 scripts/package_calculator_apk.py 2>&1 | Out-String