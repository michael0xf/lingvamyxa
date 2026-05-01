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

## Build only the C part

```bash
cmake -S . -B build -DBUILD_LINGVAMYXA_QT_APP=OFF
cmake --build build
```
