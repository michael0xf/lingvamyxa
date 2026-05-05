# lingvamyxa
Luminomyxa Proxima b, the planet's technosphere programming language

# Lingvamyxa C + optional Qt skeleton

This is a small importable CMake project.

## Structure

```text
lingvamyxa_c_qt_skeleton/
  CMakeLists.txt
  core/
    lm_core.h
    lm_core.c
  cli/
    main.c
  qt_app/
    main.cpp
    MainWindow.cpp
    MainWindow.h
```

## Idea

- `lingvamyxa_core` is a pure C static library.
- `lingvamyxa_cli` is a tiny command-line test app.
- `lingvamyxa_qt` is an optional Qt Widgets GUI wrapper.

The core does not depend on Qt, Android, Windows, POSIX, GUI, filesystem, or any event loop.

## Import into Qt Creator

1. Open Qt Creator.
2. Choose **File → Open File or Project...**
3. Select `CMakeLists.txt`.
4. Choose a desktop kit or Android kit.
5. Configure the project.

If Qt6 Widgets is available, the GUI target `lingvamyxa_qt` is created.
If Qt6 Widgets is not available, CMake skips the GUI target and still builds the pure C core and CLI.

## Command-line build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run CLI:

```bash
./build/lingvamyxa_cli
```

On Windows, the executable is usually:

```bat
build\lingvamyxa_cli.exe
```

## Windows Qt/MinGW/vcpkg build

This project can be built with the Qt-installed CMake, Ninja and MinGW tools plus vcpkg manifest dependencies.

Add Qt tools to the user `Path` once:

```powershell
$qtTools = @(
  "C:\Qt\Tools\CMake_64\bin",
  "C:\Qt\Tools\Ninja",
  "C:\Qt\Tools\mingw1310_64\bin"
)

$old = [Environment]::GetEnvironmentVariable("Path", "User")
$new = ($qtTools + ($old -split ';' | Where-Object { $_ -and ($qtTools -notcontains $_) })) -join ';'
[Environment]::SetEnvironmentVariable("Path", $new, "User")
```

Open a new PowerShell and verify:

```powershell
cmake --version
ninja --version
gcc --version
```

Install/bootstrap vcpkg once:

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat -disableMetrics
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

Configure with the MinGW vcpkg triplets:

```powershell
cmake -S C:\Nyasha_Planet\lingvamyxa `
  -B C:\Nyasha_Planet\lingvamyxa\build\mingw-vcpkg `
  -G Ninja `
  -DCMAKE_MAKE_PROGRAM=C:\Qt\Tools\Ninja\ninja.exe `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic `
  -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic `
  -DCMAKE_C_COMPILER=C:\Qt\Tools\mingw1310_64\bin\gcc.exe `
  -DCMAKE_CXX_COMPILER=C:\Qt\Tools\mingw1310_64\bin\g++.exe `
  -DBUILD_LINGVAMYXA_QT_APP=OFF
```

Build and run the CLI:

```powershell
cmake --build C:\Nyasha_Planet\lingvamyxa\build\mingw-vcpkg
C:\Nyasha_Planet\lingvamyxa\build\mingw-vcpkg\lingvamyxa_cli.exe
```

## Build only the C part

```bash
cmake -S . -B build -DBUILD_LINGVAMYXA_QT_APP=OFF
cmake --build build
```
