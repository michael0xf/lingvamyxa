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
option(LINGVAMYXA_ENABLE_EXTERNAL_DEPS "Enable optional runtime/backend dependencies" OFF)

# ---- External language/runtime dependencies ----

if (LINGVAMYXA_ENABLE_EXTERNAL_DEPS)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    find_package(pcre2 CONFIG REQUIRED)
    find_package(civetweb CONFIG REQUIRED)
    find_package(CURL CONFIG REQUIRED)
    find_package(yyjson CONFIG REQUIRED)

    find_package(PkgConfig REQUIRED)

    # bigint -> GMP mpz_t
    pkg_check_modules(gmp REQUIRED IMPORTED_TARGET gmp)

    # real -> MPFR mpfr_t
    pkg_check_modules(mpfr REQUIRED IMPORTED_TARGET mpfr)

    # decimal -> decNumber

    set(DECNUMBER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/decNumber/decNumber-icu-368")

    add_library(decnumber STATIC
        "${DECNUMBER_DIR}/decContext.c"
        "${DECNUMBER_DIR}/decNumber.c"
    )

    target_include_directories(decnumber
        PUBLIC
            "${DECNUMBER_DIR}"
    )

    target_compile_features(decnumber
        PUBLIC
            c_std_99
    )

    target_compile_definitions(decnumber
        PUBLIC
            DECNUMDIGITS=64
    )
endif()

# ---- Pure C language core ----
# This target must stay independent from Qt, Android, Windows APIs, etc.

add_library(lingvamyxa_core STATIC
    core/lm_core.c
    core/lm_core.h
    core/lm_p0.c
    core/lm_p0.h
)

target_include_directories(lingvamyxa_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

if (LINGVAMYXA_ENABLE_EXTERNAL_DEPS)
    target_compile_definitions(lingvamyxa_core
        PUBLIC
            PCRE2_CODE_UNIT_WIDTH=8
    )

    target_link_libraries(lingvamyxa_core
        PUBLIC
            unofficial::sqlite3::sqlite3
            PCRE2::8BIT
            civetweb::civetweb
            CURL::libcurl
            PkgConfig::gmp
            PkgConfig::mpfr
            yyjson::yyjson
            decnumber
    )
endif()

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

    target_include_directories(lingvamyxa_cli PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/core
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
