# lingvamyxa
Luminomyxa Proxima b, the planet's technosphere programming language

# Lingvamyxa C + optional Qt shell

This is a small importable CMake project.

## Structure

```text
lingvamyxa/
  CMakeLists.txt
  lm1/
    parser.lm1.h
  lm2/
    trans.lm2
    parser.lm2
    printTree.lm2
    make.lm2
    finalize.lm2
    buildCore.lmx
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
      buildCore.lm1.c
      parser.lm1.c
      printTree.lm1.c
      make.lm1.c
      trans.lm1.c
      finalize.lm1.c
```

## Idea

- `parser.lm1` is the L1/C parser static library.
- `printTree.lm0` is the tiny command-line tree dump app and built L0 executable.
- `trans.lm0` is the first L2-to-L1 translator executable.
- `make.lm0` is the first native build driver generated from `lm2/make.lm2`.
- `finalize.lm0` is the post-build installer that replaces `.next` tools in `build/lm0`.
- `buildCore.lm0` is the project build driver compiled from `build/lm1/buildCore.lm1.c`; after bootstrap, that snapshot is regenerated from `lm2/buildCore.lmx`.
- `build/lm0/...` is the trusted current L0 tool layer and the local L0 output directory; it is not a portable source snapshot.
- `build/libs/...` is the local native C library output/profile directory.
- `build/lm1/...` is the generated L1 mirror produced from `lm2/...`; this is the portable build snapshot kept in Git.
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

## Windows Qt/MinGW Build

This project can be built with the Qt-installed CMake, Ninja and MinGW tools. The normal Lingvamyxa build does not use the vcpkg toolchain.

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

Optionally install/bootstrap vcpkg once if you want to refresh third-party source archives:

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat -disableMetrics
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

## Download source archives with vcpkg

The source catalog in `cmake/vcpkg-sources/vcpkg.json` is only a helper for source tracking and occasional updates. It includes the external C libraries that can be imported into `third_party` and built by our own CMake rules, including `libsodium`.

```powershell
C:\Nyasha_Planet\lingvamyxa\build\lm0\vcpkgFetch.lm0.exe `
  --output C:\Nyasha_Planet\lingvamyxa\build\vcpkg-downloads
```

On Linux/macOS, use the extensionless tool:

```sh
./build/lm0/vcpkgFetch.lm0 --output ./build/vcpkg-downloads
```

This does not build the libraries for Lingvamyxa. It runs vcpkg with `--only-downloads` against `cmake/vcpkg-sources/vcpkg.json` and places the downloaded source archive cache under the selected `--downloads-root`. Selected archives can then be imported into `third_party`.
Use `--triplet`, `--host-triplet`, `--vcpkg` and `--vcpkg-root` when the defaults are not right; the same values can be supplied through `LM_VCPKG_TRIPLET`, `LM_VCPKG_HOST_TRIPLET`, `LM_VCPKG` and `VCPKG_ROOT`.

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

For the full bootstrap mode, pass `--full` to `buildCore.lm0`. This refreshes the L0 tools first, then configures and builds bundled `third_party` libraries into `build/libs` with `LINGVAMYXA_ENABLE_EXTERNAL_DEPS=OFF` and `LINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY=ON`. No vcpkg toolchain is used by this mode.

The full profile produces plain C library names in `build/libs`: `libcivetweb.a`, `libcurl.a`, `libdecnumber.a`, `libgmp.a`, `libmpfr.a`, `libpcre2-8.a`, `libpcre2-posix.a`, `libsodium.a`, `libsqlite3.a`, `libyyjson.a`, and `libz.a`.

On Windows, the bundled GMP/MPFR autotools builds use the MSYS2 shell and `make` discovered under `VCPKG_ROOT\downloads\tools\msys2` or `C:\vcpkg\downloads\tools\msys2`; that MSYS2 `usr/bin` is added only to the subprocess `PATH`. On Linux/macOS, the same rule uses the normal system `sh`/`bash` and `make`/`gmake`.

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
