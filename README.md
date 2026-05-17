# lingvamyxa
Luminomyxa Proxima b, the planet's technosphere programming language

# Lingvamyxa C + optional Qt shell

This is a small importable CMake project.

## Structure

```text
lingvamyxa/
  CMakeLists.txt
  lm1/
    parser/
      parser.lm1.h
    buildCore/
      buildCore.lm1.c
  lm2/
    trans/
      trans.lm2
    parser/
      parser.lm2
    printTree/
      printTree.lm2
    make/
      make.lm2
    finalize/
      finalize.lm2
  qt_app/
    main.cpp
    MainWindow.cpp
    MainWindow.h
  build/
    lm0/
      libparser.lm0.a
      printTree.lm0.exe
      trans.lm0.exe
      make.lm0.exe
      finalize.lm0.exe
      buildCore.lm0.exe
    lm1/
      parser/
        parser.lm1.c
      printTree/
        printTree.lm1.c
      make/
        make.lm1.c
      trans/
        trans.lm1.c
      finalize/
        finalize.lm1.c
```

## Idea

- `parser.lm1` is the L1/C parser static library.
- `printTree.lm0` is the tiny command-line tree dump app and built L0 executable.
- `trans.lm0` is the first L2-to-L1 translator executable.
- `make.lm0` is the first native build driver generated from `lm2/make/make.lm2`.
- `finalize.lm0` is the post-build installer that replaces `.next` tools in `build/lm0`.
- `buildCore.lm0` is the project build driver compiled from portable L1/C source.
- `build/lm0/...` is the trusted current L0 tool layer and the local L0 output directory.
- `build/lm1/...` is the generated L1 mirror produced from `lm2/...`; it is also the portable source snapshot for the first build on a new platform.
- `lingvamyxa_qt` is an optional Qt Widgets GUI wrapper.

## Import into Qt Creator

1. Open Qt Creator.
2. Choose **File → Open File or Project...**
3. Select `CMakeLists.txt`.
4. Choose a desktop kit or Android kit.
5. Configure the project.

If Qt6 Widgets is available, the GUI target `lingvamyxa_qt` is created.
If Qt6 Widgets is not available, CMake skips the GUI target and still builds the parser and CLI tools.

## Command-line build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run CLI:

```bash
./build/lm0/printTree.lm0
```

On Windows, the executable is usually:

```bat
build\lm0\printTree.lm0.exe
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
  -DVCPKG_MANIFEST_DIR=C:\Nyasha_Planet\lingvamyxa\cmake\vcpkg `
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic `
  -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic `
  -DCMAKE_C_COMPILER=C:\Qt\Tools\mingw1310_64\bin\gcc.exe `
  -DCMAKE_CXX_COMPILER=C:\Qt\Tools\mingw1310_64\bin\g++.exe `
  -DLINGVAMYXA_ENABLE_EXTERNAL_DEPS=ON `
  -DBUILD_LINGVAMYXA_QT_APP=OFF
```

Build and run the CLI:

```powershell
cmake --build C:\Nyasha_Planet\lingvamyxa\build\mingw-vcpkg
C:\Nyasha_Planet\lingvamyxa\build\lm0\printTree.lm0.exe
```

## Refresh trusted L0 tools

`buildCore.lm0` refreshes generated L1 sources and rebuilds the L0 tools into `build/lm0`.
`finalize.lm0` is produced by `buildCore.lm0`, not by the CMake preset.
It builds lock-sensitive tools as `.next` files, then schedules `build/lm0/finalize.lm0` to install them after `buildCore.lm0` exits:

```powershell
C:\Nyasha_Planet\lingvamyxa\build\lm0\buildCore.lm0.exe
```

From CMake or Qt Creator, use the `runBuildCore.lm0` target. It builds the CMake `buildCore.lm0` target first, then runs the trusted `build/lm0/buildCore.lm0` from the project root:

```powershell
cmake --build --preset run-buildcore
```

For the full bootstrap mode, pass `--full` to `buildCore.lm0`. This refreshes the L0 tools first, then configures and builds a separate full CMake profile in `build/full` with `cmake/vcpkg/vcpkg.json`, `LINGVAMYXA_ENABLE_EXTERNAL_DEPS=ON`, and bundled `third_party` libraries enabled. The bundled C libraries keep plain C names in `build/lm0`: `libdecnumber.a` and `libsodium.a`.

```powershell
C:\Nyasha_Planet\lingvamyxa\build\lm0\buildCore.lm0.exe --full
```

From Qt Creator, set the CMake cache variable `LINGVAMYXA_BUILDCORE_ARGS` to `--full` and build the `runBuildCore.lm0` target. From the command line, the preset shortcut is:

```powershell
cmake --build --preset run-buildcore-full
```

## Build only the C part

```bash
cmake -S . -B build -DBUILD_LINGVAMYXA_QT_APP=OFF
cmake --build build
```
