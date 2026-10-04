# APKRunner Fusion 2.0

Fusion is a Windows host launcher that delegates APK installation and app launch to Android Debug Bridge (ADB). It can install an APK on an already-running, ADB-connected Android guest and request that guest to launch the app. A GUI-capable guest may then show its app window using its own Windows integration.

Fusion does not contain Android, ART, Binder, an ARM translator, ANGLE, a hypervisor, or an Android system image. It does not boot WSL2 or provision a guest. WSL2 by itself is not an Android runtime, and a headless Android container does not automatically produce a Windows app window. The Windows host must already have a compatible Android guest with GUI/window integration running, and `adb.exe` must be available on `PATH` and connected to it. Existing WSA installations or Android guests/emulators with ADB support may be used; WSA is discontinued and is not distributed here.

No APKs or games are bundled. Installation uses `adb install -r`; launch uses Android's launcher-category `monkey` command. Guest compatibility depends on the Android version, app requirements, ABI support, and guest graphics implementation.

## Build

From the repository root, build a Linux test executable:

```sh
cmake -S apk-runner-fusion -B build-fusion-linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-fusion-linux --config Release
```

Cross-compile a standalone MinGW executable from Linux (MinGW-w64 must be installed):

```sh
cmake -S apk-runner-fusion -B build-fusion -G Ninja \\
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/mingw-toolchain.cmake" \\
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-fusion --config Release
```

The Windows executable is `build-fusion/APKRunner-Fusion.exe`. MinGW statically links its C++ runtime; Windows system libraries are supplied by the OS. No ADB binary or Android guest is bundled.

## Use on Windows

Install Android platform-tools, start and configure an Android guest with GUI integration, and connect ADB to that guest. Then:

```powershell
APKRunner-Fusion.exe devices
APKRunner-Fusion.exe install .\app-debug.apk
APKRunner-Fusion.exe run com.example.app
```

The launcher returns ADB's exit code. It does not guarantee that an app is compatible or that the selected guest can display it as an individual Windows window.