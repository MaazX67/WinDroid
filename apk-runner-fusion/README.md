# APKRunner Fusion 2.1

Fusion is a Windows host launcher that delegates APK installation and app launch to Android Debug Bridge (ADB). The Windows build ships with the official platform-tools `adb.exe`, `AdbWinApi.dll`, and `AdbWinUsbApi.dll`. It prefers ADB beside the launcher, checks `%TEMP%\APKRunnerFusion`, then searches `PATH`.

Fusion can install an APK on an already-running, ADB-connected Android guest and request that guest to launch the app. The app window is provided by the guest, if that guest supports GUI integration.

Fusion does not contain or boot Android, ART, Binder, an ARM translator, ANGLE, a hypervisor, or an Android system image. WSL2 alone is not an Android runtime, and Redroid container images are not bootable WSL distributions. No guest auto-download or first-run provisioning is implemented. A compatible Android guest must already be configured and connected. WSA is discontinued and is not bundled.

No APKs or games are bundled. Installation uses `adb install -r`; launch uses Android's launcher-category `monkey` command. On Windows, `install` asks for the package name after a successful install and can create a per-user desktop shortcut. `--register` adds Fusion to the current user's `.apk` Open With list. Guest compatibility depends on the Android version, app requirements, ABI, native libraries, and graphics implementation. There is no guarantee of support for all ARMv8a, ARMv7, x86_64, Unity, Unreal, Godot, or other APKs.

## Build

From the repository root, build a Linux test executable:

```sh
cmake -S apk-runner-fusion -B build-fusion-linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-fusion-linux --config Release
```

Cross-compile the Windows executable and copy the bundled ADB files beside it (MinGW-w64 must be installed):

```sh
cmake -S apk-runner-fusion -B build-fusion -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/mingw-toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-fusion --config Release
makensis apk-runner-fusion/installer/installer.nsi
```

Outputs: `build-fusion/APKRunner-Fusion.exe` and `build-fusion/APKRunner-Universal-Setup.exe`. MinGW statically links its C++ runtime; Windows system libraries are supplied by the OS. The setup contains the Fusion executable and ADB platform-tools files, but no Android guest.

## Use on Windows

Install and configure a compatible Android guest with ADB enabled and connect it. The setup includes ADB; no separate platform-tools installation is needed. Then:

```powershell
APKRunner-Fusion.exe devices
APKRunner-Fusion.exe install .\app-debug.apk
APKRunner-Fusion.exe run com.example.app
APKRunner-Fusion.exe --register
```

`devices` returns exit code 10 and a diagnostic if ADB is unavailable or no device is online. `--register` adds the current user-level Open With association without changing the default app. `open <APK>` installs an APK selected through that association. The installer adds the same Open With option and desktop shortcut.

Cross-compilation and setup generation are tested in Linux. The Windows installer UI, registry behavior, connected guest install, shortcut launch, and visible app window require Windows/Android runtime testing and are not verified by the Codespace build.
