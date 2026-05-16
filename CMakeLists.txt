cmake_minimum_required(VERSION 3.22)

project(lingvamyxa
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

option(BUILD_PARSER_LM0 "Build the parser.lm0 command-line parser executable" ON)
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

# ---- L1 core ----
# This target must stay independent from Qt, Android, Windows APIs, etc.

add_library(core.lm1 STATIC
    lm1/core/core.lm1.c
    lm1/core/core.lm1.h
)

set_target_properties(core.lm1 PROPERTIES
    OUTPUT_NAME "core.lm0"
)

target_include_directories(core.lm1 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lm1/core
)

if (LINGVAMYXA_ENABLE_EXTERNAL_DEPS)
    target_compile_definitions(core.lm1
        PUBLIC
            PCRE2_CODE_UNIT_WIDTH=8
    )

    target_link_libraries(core.lm1
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
    target_compile_options(core.lm1 PRIVATE /W4)
else()
    target_compile_options(core.lm1 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- L1 parser ----

add_library(parser.lm1 STATIC
    lm1/parser/p0.lm1.c
    lm1/parser/p0.lm1.h
)

set_target_properties(parser.lm1 PROPERTIES
    OUTPUT_NAME "parser.lm0"
)

target_include_directories(parser.lm1 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lm1/parser
)

target_link_libraries(parser.lm1 PUBLIC
    core.lm1
)

if (MSVC)
    target_compile_options(parser.lm1 PRIVATE /W4)
else()
    target_compile_options(parser.lm1 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- L0 parser executable ----

if (BUILD_PARSER_LM0)
    add_executable(parser.lm0
        lm1/parser/parser.lm1.c
    )

    target_link_libraries(parser.lm0 PRIVATE
        parser.lm1
    )
endif()

# ---- Optional Qt Widgets shell ----
# If Qt6 Widgets is available, this builds a tiny GUI wrapper around the same C core.
# If Qt is not available, the project still configures and builds the L1 core
# and parser targets.

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
            core.lm1
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
