# APKRunner-Universal

![Build status](https://img.shields.io/badge/build-passing-brightgreen)

## Build status

Verified in this Codespace: `./gradlew assembleDebug`, the root CMake inspector build and APK detection, the MinGW Windows Fusion build, and NSIS setup generation. The Android APK passed package/archive checks. Windows registry, installer UI, connected-device install, and visible guest-window behavior still require validation on a Windows host with a configured Android guest.

## Install WinDroid APK

Install with Android platform-tools and a connected Android device or emulator:

```sh
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell monkey -p com.windroid -c android.intent.category.LAUNCHER 1
```

## Install Fusion setup

Download `APKRunner-Universal-Setup.exe` from the [v6.0 release](https://github.com/MaazX67/WinDroid/releases/tag/v6.0-full-functional) and run it. The per-user installer places Fusion and the bundled ADB files under `%LOCALAPPDATA%\Programs\APKRunnerFusion`, adds a desktop shortcut, and registers Fusion as an option in the `.apk` Open With list. It does not install or provision an Android guest.

## Use Open With

In Windows Explorer, right-click an APK, choose **Open with**, and select **APKRunner Fusion**. Fusion sends `adb install -r` to the connected guest. It then asks for the Android package name if you want a desktop launch shortcut. The registration can also be requested directly with `APKRunner-Fusion.exe --register`.

## Fusion host bridge

Fusion is an ADB host client, not an APK compatibility runtime. It does not include Android, ART, Binder, Box64/FEX, ANGLE, WSL images, or a guest OS. Android guests must already be running and connected through ADB; displaying an app in a separate Windows window depends on the guest's own integration. Redroid container images are not bootable WSL distributions, so Fusion does not claim to download or boot them.

There is no guarantee that all ARMv8a, ARMv7, x86_64, Unity, Unreal, Godot, or other APKs will run. The guest must support the APK's Android API level, ABI, native libraries, services, and graphics requirements. Use a guest/emulator that supports the app; performance and compatibility are guest-dependent.

See [`apk-runner-fusion/README.md`](apk-runner-fusion/README.md) for build and command details.

## Run the CLI

Inspect an APK on Linux:

```sh
./build/APKRunner detect path/to/app.apk
```

Install and launch an APK through the bundled ADB client and a connected guest on Windows:

```powershell
.\build-fusion\APKRunner-Fusion.exe devices
.\build-fusion\APKRunner-Fusion.exe install .\app.apk
.\build-fusion\APKRunner-Fusion.exe run com.example.app
```

APKRunner-Universal is an early native C++ project for inspecting Android APK archives. Its current executable detects likely engine markers, checks for a manifest and DEX files, and lists common Android native ABIs. It does **not** execute APKs or produce standalone Windows executables.

## Important technical scope

An APK is not just DEX plus shared libraries. Android apps depend on ART and Android framework APIs, process and Binder services, Bionic behavior, Android resource/package semantics, and platform graphics, audio, and input systems. A Windows executable cannot supply those contracts by translating file formats alone. `dex2oat` is part of the Android runtime and does not turn an arbitrary APK into a self-contained Windows DLL.

Box64 is a Linux userspace compatibility layer for running x86-64 Linux programs on supported ARM Linux systems. It is not an Android ELF loader for Windows and does not provide Bionic or Android framework behavior. Accordingly, this starter reports native ABI metadata but does not claim to load `.so` files. Engine detection is heuristic and is not proof that an APK can run.

The current `run` and `build` commands deliberately return an unavailable status instead of presenting a nonfunctional launcher as a runner. No games or game-specific files are bundled.

## Build on Ubuntu or Codespaces

Install CMake, a C++17 compiler, pkg-config, and libzip development files. In the included dev container these are provisioned for you.

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

If Ninja is not installed, omit `-G Ninja` to use the platform's default generator. The executable is `build/APKRunner` on Linux and `build/APKRunner.exe` on Windows.

## Build on Windows

Use Visual Studio 2022 with the C++ desktop workload and install libzip through vcpkg:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

The vcpkg manifest installs libzip. CMake also supports a system libzip found by pkg-config on Linux.

## Cross-compile from Linux

Install the MinGW-w64 compiler and a Windows-targeted libzip dependency. The provided toolchain selects the compiler; it does not build or provide third-party cross-compiled libraries. When using vcpkg, use its MinGW triplet and chainload the compiler toolchain as appropriate for that vcpkg installation.

```sh
cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-toolchain.cmake
cmake --build build-win
```

## Commands

```sh
./build/APKRunner detect path/to/app.apk
./build/APKRunner run path/to/app.apk --width 1280 --height 720 --fps 60
./build/APKRunner build path/to/app.apk -o app.exe
```

`detect` reads APK ZIP entry names only; it does not extract or execute APK content. `run` and `build` currently explain that their runtime backends are unavailable and exit with status 3.

## Development status

The implementation is intentionally limited to archive metadata and heuristic engine classification. Before any execution claim is reasonable, the project needs a supported application compatibility scope and real implementations or upstream integrations for Android runtime/framework APIs, JNI, native ABI compatibility, windowing/rendering, audio, input, lifecycle, and packaging. Supporting arbitrary current and future APKs without per-app compatibility work is not a credible guarantee for this architecture.