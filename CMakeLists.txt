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
set(LM_AUTOTOOLS_JOBS "2" CACHE STRING "Parallel jobs used when building bundled autotools libraries")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(LingvamyxaThirdParty)

add_custom_target(extractThirdPartyZips.lm0
    COMMAND ${CMAKE_COMMAND}
        "-DLM_THIRD_PARTY_ARCHIVE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/third_party"
        "-DLM_THIRD_PARTY_EXTRACT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/build/third_party"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExtractThirdPartyZips.cmake"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Extracting third_party/*.zip archives"
    USES_TERMINAL
    VERBATIM
)

option(BUILD_PRINT_TREE_LM0 "Build the printTree.lm0 tree dump executable" ON)
option(BUILD_TRANS_LM0 "Build the trans.lm0 L2-to-L1 translator executable" ON)
option(BUILD_MAKE_LM0 "Build the make.lm0 native build driver executable" ON)
option(BUILD_VCPKG_FETCH_LM0 "Build the vcpkgFetch.lm0 source archive downloader executable" ON)
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

if (LINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY)
    lm_register_bundled_third_party(LM_BUNDLED_THIRD_PARTY_TARGETS)
    list(APPEND LM_THIRD_PARTY_TARGETS ${LM_BUNDLED_THIRD_PARTY_TARGETS})
endif()

if (LM_THIRD_PARTY_TARGETS)
    add_custom_target(third_party.lm0 ALL
        DEPENDS ${LM_THIRD_PARTY_TARGETS}
    )
endif()

set(LM1_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lm1/build")
set(LM1_GENERATED_OWN_C "${LM1_GENERATED_DIR}/own.lm1.c")
set(LM1_GENERATED_TRANS_C "${LM1_GENERATED_DIR}/trans.lm1.c")
set(LM1_GENERATED_PARSER_C "${LM1_GENERATED_DIR}/parser.lm1.c")
set(LM1_GENERATED_PRINT_TREE_C "${LM1_GENERATED_DIR}/printTree.lm1.c")
set(LM1_GENERATED_MAKE_C "${LM1_GENERATED_DIR}/make.lm1.c")
set(LM1_GENERATED_VCPKG_FETCH_C "${LM1_GENERATED_DIR}/vcpkgFetch.lm1.c")
set(LM1_GENERATED_BUILD_CORE_C "${LM1_GENERATED_DIR}/buildCore.lm1.c")
set(LM2_CORE_REGISTRY "${CMAKE_CURRENT_SOURCE_DIR}/lm2/core.lm2")
set(LM2_TRANS_REGISTRY "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans_registry.lm2")
set(LM2_PARSER_REGISTRY "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser_registry.lm2")
set(LM2_PARSER_TEXT "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser_text.lm2")
set(LM2_PARSER_REGISTRY_SELFTEST "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser_registry_selftest.lm2")
set(LM2_OWN_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/own_abi.lm2")
set(LM2_PARSER_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser_abi.lm2")
set(LM2_L4_LOADER_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/l4_loader_abi.lm2")
set(LM2_TRANS_INTERNAL_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans_internal_abi.lm2")
set(LM2_LIBC_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/libc_abi.lm2")
set(LM2_L4_LOADER_UNIT_PAYLOAD "${CMAKE_CURRENT_SOURCE_DIR}/lm2/l4_loader_unit_payload.lm2")
set(LM2_TABLE_DESCRIPTOR "${CMAKE_CURRENT_SOURCE_DIR}/lm2/table_descriptor.lm2")
set(LM2_TABLE_DESCRIPTOR_MODEL "${CMAKE_CURRENT_SOURCE_DIR}/lm2/table_descriptor_model.lm2")
set(LM2_TO_LM1_REGISTRY_DEPS
    "${LM2_CORE_REGISTRY}"
    "${LM2_TRANS_REGISTRY}"
    "${LM2_PARSER_REGISTRY}"
    "${LM2_PARSER_TEXT}"
    "${LM2_PARSER_REGISTRY_SELFTEST}"
    "${LM2_OWN_ABI}"
    "${LM2_PARSER_ABI}"
    "${LM2_L4_LOADER_ABI}"
    "${LM2_TRANS_INTERNAL_ABI}"
    "${LM2_LIBC_ABI}"
    "${LM2_L4_LOADER_UNIT_PAYLOAD}"
    "${LM2_TABLE_DESCRIPTOR}"
    "${LM2_TABLE_DESCRIPTOR_MODEL}"
)

# ---- Generated L1 ownership helper ----

if (BUILD_TRANS_LM0 OR BUILD_PRINT_TREE_LM0 OR BUILD_VCPKG_FETCH_LM0 OR BUILD_BUILD_CORE_LM0)
    add_library(own.lm1 STATIC
        "${LM1_GENERATED_OWN_C}"
    )

    set_target_properties(own.lm1 PROPERTIES
        OUTPUT_NAME "own.lm0"
    )

    target_include_directories(own.lm1 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1
    )

    if (MSVC)
        target_compile_options(own.lm1 PRIVATE /W4)
    else()
        target_compile_options(own.lm1 PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# ---- Generated L1 parser snapshot ----

if (BUILD_TRANS_LM0 OR BUILD_PRINT_TREE_LM0)
    add_library(parser.lm1 STATIC
        "${LM1_GENERATED_PARSER_C}"
    )

    set_target_properties(parser.lm1 PROPERTIES
        OUTPUT_NAME "parser.lm0"
    )

    target_include_directories(parser.lm1 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1
    )

    target_link_libraries(parser.lm1 PUBLIC
        own.lm1
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
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1
    )

    target_link_libraries(trans.lm0 PRIVATE
        parser.lm1
        own.lm1
    )

    add_custom_target(refresh_own_lm1
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/own.lm2"
            "${LM1_GENERATED_OWN_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/own.lm2"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Refreshing lm1/build/own.lm1.c from lm2/own.lm2"
        VERBATIM
    )

    add_custom_target(refresh_trans_lm1
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans.lm2"
            "${LM1_GENERATED_TRANS_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans.lm2"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Refreshing lm1/build/trans.lm1.c from lm2/trans.lm2"
        VERBATIM
    )

    add_custom_target(refresh_parser_lm1
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser.lm2"
            "${LM1_GENERATED_PARSER_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser.lm2"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Refreshing lm1/build/parser.lm1.c from lm2/parser.lm2"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LM1_GENERATED_PRINT_TREE_C}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/printTree.lm2"
            "${LM1_GENERATED_PRINT_TREE_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/printTree.lm2"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Generating lm1/build/printTree.lm1.c from lm2/printTree.lm2"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LM1_GENERATED_MAKE_C}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/make.lm2"
            "${LM1_GENERATED_MAKE_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/make.lm2"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Generating lm1/build/make.lm1.c from lm2/make.lm2"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LM1_GENERATED_VCPKG_FETCH_C}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/vcpkgFetch.lm2"
            "${LM1_GENERATED_VCPKG_FETCH_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/vcpkgFetch.lm2"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Generating lm1/build/vcpkgFetch.lm1.c from lm2/vcpkgFetch.lm2"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${LM1_GENERATED_BUILD_CORE_C}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/buildCore.lmx"
            "${LM1_GENERATED_BUILD_CORE_C}"
        DEPENDS
            trans.lm0
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/buildCore.lmx"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Generating lm1/build/buildCore.lm1.c from lm2/buildCore.lmx"
        VERBATIM
    )

    add_custom_target(lm2_to_lm1
        DEPENDS
            refresh_own_lm1
            refresh_trans_lm1
            refresh_parser_lm1
            "${LM1_GENERATED_PRINT_TREE_C}"
            "${LM1_GENERATED_MAKE_C}"
            "${LM1_GENERATED_VCPKG_FETCH_C}"
            "${LM1_GENERATED_BUILD_CORE_C}"
    )
endif()

# ---- L0 native build driver ----

if (BUILD_MAKE_LM0)
    add_executable(make.lm0
        "${LM1_GENERATED_MAKE_C}"
    )
endif()

# ---- L0 vcpkg source downloader ----

if (BUILD_VCPKG_FETCH_LM0)
    add_executable(vcpkgFetch.lm0
        "${LM1_GENERATED_VCPKG_FETCH_C}"
    )

    target_include_directories(vcpkgFetch.lm0 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1
    )

    target_link_libraries(vcpkgFetch.lm0 PRIVATE
        own.lm1
    )
endif()

# ---- L0 core build driver ----

if (BUILD_BUILD_CORE_LM0)
    add_executable(buildCore.lm0
        "${LM1_GENERATED_BUILD_CORE_C}"
    )

    target_include_directories(buildCore.lm0 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1
    )

    target_link_libraries(buildCore.lm0 PRIVATE
        own.lm1
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
