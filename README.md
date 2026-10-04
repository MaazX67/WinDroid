# APKRunner-Universal

## Fusion host bridge

The separate [`apk-runner-fusion`](apk-runner-fusion/README.md) target builds a Windows CLI that uses an existing ADB-connected Android guest to install an APK and request app launch. It does not bundle or boot Android, implement ART/Binder/ARM translation, or turn APKs into standalone Windows apps. A compatible Android guest, ADB platform-tools, and GUI/window integration must already be available on Windows.

## Build status

Verified in the Codespace: `./gradlew assembleDebug` builds the Android debug APK, the root CMake target builds the APK metadata inspector, and MinGW cross-compiles `build-fusion/APKRunner-Fusion.exe`. The APK build confirms packaging only; no Android device was available here to verify app runtime behavior. Fusion's install and launch commands require a separately installed ADB client and an already-running Android guest.

## Install the Android app

With Android platform-tools installed and an Android device connected over ADB:

```sh
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell monkey -p com.windroid -c android.intent.category.LAUNCHER 1
```

## Run the CLI

Inspect an APK on Linux:

```sh
./build/APKRunner detect path/to/app.apk
```

Install and launch an APK through an ADB-connected guest on Windows:

```powershell
.\\build-fusion\\APKRunner-Fusion.exe install .\\app.apk
.\\build-fusion\\APKRunner-Fusion.exe run com.example.app
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