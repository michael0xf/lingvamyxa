cmake_minimum_required(VERSION 3.22)

project(lingvamyxa
    VERSION 0.1.0
    LANGUAGES C CXX
)

# ---- Language settings ----

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LM0_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/lm0")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LM0_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LM0_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${LM0_OUTPUT_DIR}")

set(LM0_TRUSTED_DIR "${LM0_OUTPUT_DIR}")
set(LM_LIBS_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/libs")

option(BUILD_PRINT_TREE_LM0 "Build the printTree.lm0 tree dump executable" ON)
option(BUILD_TRANS_LM0 "Build the trans.lm0 L2-to-L1 translator executable" ON)
option(BUILD_MAKE_LM0 "Build the make.lm0 native build driver executable" ON)
option(BUILD_BUILD_CORE_LM0 "Build the buildCore.lm0 core build driver executable" ON)
option(BUILD_LINGVAMYXA_QT_APP "Build the optional Qt Widgets shell" ON)
option(LINGVAMYXA_ENABLE_EXTERNAL_DEPS "Enable optional runtime/backend dependencies" OFF)
option(LINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY "Register bundled third_party projects in the full build profile" OFF)
set(LINGVAMYXA_BUILDCORE_ARGS "" CACHE STRING "Arguments passed to build/lm0/buildCore.lm0 by the runBuildCore.lm0 target")

# ---- External language/runtime dependencies ----

set(LM_THIRD_PARTY_TARGETS "")

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

    add_custom_target(vcpkg_deps.lm0
        COMMAND ${CMAKE_COMMAND} -E echo "vcpkg manifest dependencies are available in the active vcpkg_installed tree"
        VERBATIM
    )

    list(APPEND LM_THIRD_PARTY_TARGETS vcpkg_deps.lm0)
endif()

if (LINGVAMYXA_ENABLE_EXTERNAL_DEPS OR LINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY)
    # decimal -> decNumber
    set(DECNUMBER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/decNumber/decNumber-icu-368")

    add_library(decnumber STATIC
        "${DECNUMBER_DIR}/decContext.c"
        "${DECNUMBER_DIR}/decNumber.c"
    )

    set_target_properties(decnumber PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${LM_LIBS_OUTPUT_DIR}"
        OUTPUT_NAME "decnumber"
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

    list(APPEND LM_THIRD_PARTY_TARGETS decnumber)
endif()

if (LINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY)
    set(LIBSODIUM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium-1.0.22")

    if (NOT EXISTS "${LIBSODIUM_DIR}/src/libsodium/include/sodium.h")
        message(FATAL_ERROR "Bundled libsodium source was not found: ${LIBSODIUM_DIR}")
    endif()

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/libsodium/sodium")
    configure_file(
        "${LIBSODIUM_DIR}/builds/msvc/version.h"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/libsodium/sodium/version.h"
        COPYONLY
    )

    file(GLOB_RECURSE LIBSODIUM_SOURCES CONFIGURE_DEPENDS
        "${LIBSODIUM_DIR}/src/libsodium/*.c"
    )

    add_library(libsodium_vendor STATIC
        ${LIBSODIUM_SOURCES}
    )

    set_target_properties(libsodium_vendor PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${LM_LIBS_OUTPUT_DIR}"
        OUTPUT_NAME "sodium"
    )

    target_include_directories(libsodium_vendor
        PUBLIC
            "${CMAKE_CURRENT_BINARY_DIR}/generated/libsodium"
            "${LIBSODIUM_DIR}/src/libsodium/include"
        PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/generated/libsodium/sodium"
            "${LIBSODIUM_DIR}/src/libsodium/include/sodium"
    )

    target_compile_features(libsodium_vendor
        PUBLIC
            c_std_99
    )

    target_compile_definitions(libsodium_vendor
        PUBLIC
            SODIUM_STATIC=1
        PRIVATE
            CONFIGURED=1
            DEV_MODE=1
            HAVE_ATOMIC_OPS=1
            HAVE_C11_MEMORY_FENCES=1
            HAVE_GCC_MEMORY_FENCES=1
            HAVE_INLINE_ASM=1
            HAVE_INTTYPES_H=1
            HAVE_STDINT_H=1
            HAVE_TI_MODE=1
    )

    if (CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
        target_compile_definitions(libsodium_vendor PRIVATE NATIVE_BIG_ENDIAN=1)
    else()
        target_compile_definitions(libsodium_vendor PRIVATE NATIVE_LITTLE_ENDIAN=1)
    endif()

    if (WIN32)
        target_compile_definitions(libsodium_vendor
            PRIVATE
                HAVE_RAISE=1
                HAVE_SYS_PARAM_H=1
        )
        target_link_libraries(libsodium_vendor
            INTERFACE
                advapi32
        )
    endif()

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
        target_compile_definitions(libsodium_vendor
            PRIVATE
                HAVE_AVX2INTRIN_H=1
                HAVE_AVX512FINTRIN_H=1
                HAVE_AVXINTRIN_H=1
                HAVE_CPUID=1
                HAVE_EMMINTRIN_H=1
                HAVE_MMINTRIN_H=1
                HAVE_PMMINTRIN_H=1
                HAVE_RDRAND=1
                HAVE_SMMINTRIN_H=1
                HAVE_TMMINTRIN_H=1
                HAVE_WMMINTRIN_H=1
        )
    endif()

    if (NOT MSVC)
        target_compile_options(libsodium_vendor
            PRIVATE
                -fvisibility=hidden
                -fno-strict-aliasing
                -fno-strict-overflow
                -fwrapv
                -flax-vector-conversions
                -Wno-attributes
                -Werror=vla
        )
    endif()

    list(APPEND LM_THIRD_PARTY_TARGETS libsodium_vendor)
endif()

if (LM_THIRD_PARTY_TARGETS)
    add_custom_target(third_party.lm0 ALL
        DEPENDS ${LM_THIRD_PARTY_TARGETS}
    )
endif()

set(LM1_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/lm1")
set(LM1_GENERATED_TRANS_C "${LM1_GENERATED_DIR}/trans/trans.lm1.c")
set(LM1_GENERATED_PARSER_C "${LM1_GENERATED_DIR}/parser/parser.lm1.c")
set(LM1_GENERATED_PRINT_TREE_C "${LM1_GENERATED_DIR}/printTree/printTree.lm1.c")
set(LM1_GENERATED_MAKE_C "${LM1_GENERATED_DIR}/make/make.lm1.c")
set(LM1_GENERATED_BUILD_CORE_C "${LM1_GENERATED_DIR}/buildCore/buildCore.lm1.c")

# ---- Generated L1 parser snapshot ----

if (BUILD_TRANS_LM0 OR BUILD_PRINT_TREE_LM0)
    add_library(parser.lm1 STATIC
        "${LM1_GENERATED_PARSER_C}"
        "${CMAKE_CURRENT_SOURCE_DIR}/lm1/parser/parser.lm1.h"
    )

    set_target_properties(parser.lm1 PROPERTIES
        OUTPUT_NAME "parser.lm0"
    )

    target_include_directories(parser.lm1 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1/parser
    )

    if (MSVC)
        target_compile_options(parser.lm1 PRIVATE /W4)
    else()
        target_compile_options(parser.lm1 PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# ---- L2 to L1 translator and generated L1 mirror ----

if (BUILD_TRANS_LM0)
    add_executable(trans.lm0
        "${LM1_GENERATED_TRANS_C}"
    )

    target_include_directories(trans.lm0 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1/parser
    )

    target_link_libraries(trans.lm0 PRIVATE
        parser.lm1
    )

    add_custom_target(refresh_trans_lm1
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}/trans"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans/trans.lm2"
            "${LM1_GENERATED_TRANS_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans/trans.lm2"
        COMMENT "Refreshing build/lm1/trans/trans.lm1.c from lm2/trans/trans.lm2"
        VERBATIM
    )

    add_custom_target(refresh_parser_lm1
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}/parser"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser/parser.lm2"
            "${LM1_GENERATED_PARSER_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser/parser.lm2"
        COMMENT "Refreshing build/lm1/parser/parser.lm1.c from lm2/parser/parser.lm2"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LM1_GENERATED_PRINT_TREE_C}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}/printTree"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/printTree/printTree.lm2"
            "${LM1_GENERATED_PRINT_TREE_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/printTree/printTree.lm2"
        COMMENT "Generating build/lm1/printTree/printTree.lm1.c from lm2/printTree/printTree.lm2"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LM1_GENERATED_MAKE_C}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}/make"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/make/make.lm2"
            "${LM1_GENERATED_MAKE_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/make/make.lm2"
        COMMENT "Generating build/lm1/make/make.lm1.c from lm2/make/make.lm2"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LM1_GENERATED_BUILD_CORE_C}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}/buildCore"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/buildCore/buildCore.lm2"
            "${LM1_GENERATED_BUILD_CORE_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/buildCore/buildCore.lm2"
        COMMENT "Generating build/lm1/buildCore/buildCore.lm1.c from lm2/buildCore/buildCore.lm2"
        VERBATIM
    )

    add_custom_target(lm2_to_lm1
        DEPENDS
            refresh_trans_lm1
            refresh_parser_lm1
            "${LM1_GENERATED_PRINT_TREE_C}"
            "${LM1_GENERATED_MAKE_C}"
            "${LM1_GENERATED_BUILD_CORE_C}"
    )
endif()

# ---- L0 native build driver ----

if (BUILD_MAKE_LM0)
    add_executable(make.lm0
        "${LM1_GENERATED_MAKE_C}"
    )
endif()

# ---- L0 core build driver ----

if (BUILD_BUILD_CORE_LM0)
    add_executable(buildCore.lm0
        "${LM1_GENERATED_BUILD_CORE_C}"
    )

    set(LM_RUN_BUILD_CORE_DEPS buildCore.lm0)
    if (TARGET make.lm0)
        list(APPEND LM_RUN_BUILD_CORE_DEPS make.lm0)
    endif()
    if (TARGET trans.lm0)
        list(APPEND LM_RUN_BUILD_CORE_DEPS trans.lm0)
    endif()
    if (TARGET parser.lm1)
        list(APPEND LM_RUN_BUILD_CORE_DEPS parser.lm1)
    endif()

    add_custom_target(runBuildCore.lm0
        COMMAND ${CMAKE_COMMAND} -E env
            "LM_CMAKE=${CMAKE_COMMAND}"
            "LM_CMAKE_GENERATOR=${CMAKE_GENERATOR}"
            "LM_CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
            "LM_CC=${CMAKE_C_COMPILER}"
            "LM_CXX=${CMAKE_CXX_COMPILER}"
            "LM_AR=${CMAKE_AR}"
            "LM_RANLIB=${CMAKE_RANLIB}"
            "${LM0_TRUSTED_DIR}/buildCore.lm0${CMAKE_EXECUTABLE_SUFFIX}"
            ${LINGVAMYXA_BUILDCORE_ARGS}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS ${LM_RUN_BUILD_CORE_DEPS}
        COMMENT "Running build/lm0/buildCore.lm0 from the project root"
        USES_TERMINAL
        VERBATIM
    )
endif()

# ---- L0 tree dump executable ----

if (BUILD_PRINT_TREE_LM0)
    add_executable(printTree.lm0
        "${LM1_GENERATED_PRINT_TREE_C}"
    )

    target_link_libraries(printTree.lm0 PRIVATE
        parser.lm1
    )
endif()

# ---- Optional Qt Widgets shell ----
# If Qt6 Widgets is available, this builds a tiny GUI shell.
# If Qt is not available, the project still configures and builds the parser
# and tool targets.

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
        )

        set_target_properties(lingvamyxa_qt PROPERTIES
            WIN32_EXECUTABLE TRUE
            MACOSX_BUNDLE TRUE
        )
    else()
        message(STATUS "Qt6 Widgets was not found. Skipping lingvamyxa_qt target.")
        message(STATUS "The parser and tool targets can still be built.")
    endif()
endif()
