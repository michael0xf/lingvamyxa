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

set(LM0_TRUSTED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lm0")
set(LM0_TRUSTED_PARSER_LIB "${LM0_TRUSTED_DIR}/libparser.lm0.a")

option(BUILD_PRINT_TREE_LM0 "Build the printTree.lm0 tree dump executable" ON)
option(BUILD_TRANS_LM0 "Build the trans.lm0 L2-to-L1 translator executable" ON)
option(BUILD_MAKE_LM0 "Build the make.lm0 native build driver executable" ON)
option(BUILD_BUILD_CORE_LM0 "Build the buildCore.lm0 core build driver executable" ON)
option(BUILD_LINGVAMYXA_QT_APP "Build the optional Qt Widgets shell" ON)
option(LINGVAMYXA_ENABLE_EXTERNAL_DEPS "Enable optional runtime/backend dependencies" OFF)

if (BUILD_PRINT_TREE_LM0 AND NOT BUILD_TRANS_LM0)
    message(FATAL_ERROR "BUILD_PRINT_TREE_LM0 requires BUILD_TRANS_LM0 because printTree.lm0 is generated from lm2/printTree/printTree.lm2.")
endif()

if (BUILD_MAKE_LM0 AND NOT BUILD_TRANS_LM0)
    message(FATAL_ERROR "BUILD_MAKE_LM0 requires BUILD_TRANS_LM0 because make.lm0 is generated from lm2/make/make.lm2.")
endif()

if (BUILD_TRANS_LM0 AND NOT EXISTS "${LM0_TRUSTED_PARSER_LIB}")
    message(FATAL_ERROR "Trusted parser library was not found: ${LM0_TRUSTED_PARSER_LIB}")
endif()

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

set(LM1_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/lm1")
set(LM1_GENERATED_TRANS_C "${LM1_GENERATED_DIR}/trans/trans.lm1.c")
set(LM1_GENERATED_PARSER_C "${LM1_GENERATED_DIR}/parser/parser.lm1.c")
set(LM1_GENERATED_PRINT_TREE_C "${LM1_GENERATED_DIR}/printTree/printTree.lm1.c")
set(LM1_GENERATED_MAKE_C "${LM1_GENERATED_DIR}/make/make.lm1.c")

# ---- L2 to L1 translator and generated L1 mirror ----

if (BUILD_TRANS_LM0)
    add_library(parser_trusted.lm0 STATIC IMPORTED GLOBAL)
    set_target_properties(parser_trusted.lm0 PROPERTIES
        IMPORTED_LOCATION "${LM0_TRUSTED_PARSER_LIB}"
    )

    set_source_files_properties("${LM1_GENERATED_TRANS_C}" PROPERTIES GENERATED TRUE)

    add_executable(trans.lm0
        "${LM1_GENERATED_TRANS_C}"
    )

    target_include_directories(trans.lm0 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1/parser
    )

    target_link_libraries(trans.lm0 PRIVATE
        parser_trusted.lm0
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

    add_custom_target(lm2_to_lm1 ALL
        DEPENDS
            refresh_trans_lm1
            refresh_parser_lm1
            "${LM1_GENERATED_PRINT_TREE_C}"
            "${LM1_GENERATED_MAKE_C}"
    )
endif()

# ---- Generated L1 parser ----

if (BUILD_TRANS_LM0)
    set_source_files_properties("${LM1_GENERATED_PARSER_C}" PROPERTIES GENERATED TRUE)

    add_library(parser.lm1 STATIC
        "${LM1_GENERATED_PARSER_C}"
        "${CMAKE_CURRENT_SOURCE_DIR}/lm1/parser/parser.lm1.h"
    )

    add_dependencies(parser.lm1
        lm2_to_lm1
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

# ---- L0 native build driver ----

if (BUILD_MAKE_LM0)
    set_source_files_properties("${LM1_GENERATED_MAKE_C}" PROPERTIES GENERATED TRUE)

    add_executable(make.lm0
        "${LM1_GENERATED_MAKE_C}"
    )

    add_dependencies(make.lm0
        lm2_to_lm1
    )
endif()

# ---- L0 core build driver ----

if (BUILD_BUILD_CORE_LM0)
    add_executable(buildCore.lm0
        "${CMAKE_CURRENT_SOURCE_DIR}/lm1/buildCore/buildCore.lm1.c"
    )
endif()

# ---- L0 tree dump executable ----

if (BUILD_PRINT_TREE_LM0)
    set_source_files_properties("${LM1_GENERATED_PRINT_TREE_C}" PROPERTIES GENERATED TRUE)

    add_executable(printTree.lm0
        "${LM1_GENERATED_PRINT_TREE_C}"
    )

    add_dependencies(printTree.lm0
        lm2_to_lm1
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
