cmake_minimum_required(VERSION 3.22)

project(lingvamyxa_skeleton
    VERSION 0.1.0
    LANGUAGES C CXX
)

# ---- C core settings ----

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_LINGVAMYXA_CLI "Build the simple command-line test app" ON)
option(BUILD_LINGVAMYXA_QT_APP "Build the optional Qt Widgets shell" ON)

# ---- Pure C language core ----
# This target must stay independent from Qt, Android, Windows APIs, etc.

add_library(lingvamyxa_core STATIC
    core/lm_core.c
)

target_include_directories(lingvamyxa_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

if (MSVC)
    target_compile_options(lingvamyxa_core PRIVATE /W4)
else()
    target_compile_options(lingvamyxa_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- Tiny CLI test app ----
# Useful for testing the C core without GUI or platform-specific APIs.

if (BUILD_LINGVAMYXA_CLI)
    add_executable(lingvamyxa_cli
        cli/main.c
    )

    target_link_libraries(lingvamyxa_cli PRIVATE
        lingvamyxa_core
    )
endif()

# ---- Optional Qt Widgets shell ----
# If Qt6 Widgets is available, this builds a tiny GUI wrapper around the same C core.
# If Qt is not available, the project still configures and builds the C core/CLI.

if (BUILD_LINGVAMYXA_QT_APP)
    find_package(Qt6 COMPONENTS Widgets QUIET)

    if (Qt6_FOUND)
        qt_standard_project_setup()

        qt_add_executable(lingvamyxa_qt
            qt_app/main.cpp
            qt_app/MainWindow.cpp
            qt_app/MainWindow.h
        )

        target_link_libraries(lingvamyxa_qt PRIVATE
            Qt6::Widgets
            lingvamyxa_core
        )

        set_target_properties(lingvamyxa_qt PROPERTIES
            WIN32_EXECUTABLE TRUE
            MACOSX_BUNDLE TRUE
        )
    else()
        message(STATUS "Qt6 Widgets was not found. Skipping lingvamyxa_qt target.")
        message(STATUS "The pure C core and CLI target can still be built.")
    endif()
endif()
