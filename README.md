```sh
meson setup build --prefix=/usr --buildtype=release
# or full build with webview enabled
meson setup build --prefix=/usr --buildtype=release -Denable_webview=true
meson compile -C build
meson install -C build
```

