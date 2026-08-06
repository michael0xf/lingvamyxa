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
set(LM_THREAD_PROVIDER "auto" CACHE STRING "Host thread provider: auto, pthread, win32, or single")
set_property(CACHE LM_THREAD_PROVIDER PROPERTY STRINGS auto pthread win32 single)
set(LM_REST_LMX_CLIENT_PROVIDER "auto" CACHE STRING
    "REST/LMX client provider: auto, none, libcurl, or winhttp"
)
set_property(CACHE LM_REST_LMX_CLIENT_PROVIDER PROPERTY STRINGS auto none libcurl winhttp)
set(LM_REST_LMX_SERVER_PROVIDER "auto" CACHE STRING
    "REST/LMX server provider: auto, none, or civetweb"
)
set_property(CACHE LM_REST_LMX_SERVER_PROVIDER PROPERTY STRINGS auto none civetweb)
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

# ---- Host thread provider ----

string(TOLOWER "${LM_THREAD_PROVIDER}" LM_THREAD_PROVIDER_REQUESTED)
set(LM_THREAD_PROVIDER_VALUES auto pthread win32 single)
if (NOT LM_THREAD_PROVIDER_REQUESTED IN_LIST LM_THREAD_PROVIDER_VALUES)
    message(FATAL_ERROR
        "Unsupported LM_THREAD_PROVIDER='${LM_THREAD_PROVIDER}'. "
        "Expected one of: auto, pthread, win32, single."
    )
endif()

if (LM_THREAD_PROVIDER_REQUESTED STREQUAL "auto")
    if (WIN32)
        set(LM_THREAD_PROVIDER_RESOLVED win32)
    elseif (UNIX)
        set(LM_THREAD_PROVIDER_RESOLVED pthread)
    else()
        set(LM_THREAD_PROVIDER_RESOLVED single)
    endif()
else()
    set(LM_THREAD_PROVIDER_RESOLVED "${LM_THREAD_PROVIDER_REQUESTED}")
endif()

if (LM_THREAD_PROVIDER_RESOLVED STREQUAL "pthread" AND NOT UNIX)
    message(FATAL_ERROR "LM_THREAD_PROVIDER=pthread requires a Unix-like target")
elseif (LM_THREAD_PROVIDER_RESOLVED STREQUAL "win32" AND NOT WIN32)
    message(FATAL_ERROR "LM_THREAD_PROVIDER=win32 requires a Windows target")
endif()

string(TOUPPER "${LM_THREAD_PROVIDER_RESOLVED}" LM_THREAD_PROVIDER_DEFINE_SUFFIX)
add_library(lm_thread_provider INTERFACE)
target_compile_definitions(lm_thread_provider INTERFACE
    "LM_THREAD_PROVIDER=LM_THREAD_PROVIDER_${LM_THREAD_PROVIDER_DEFINE_SUFFIX}"
)

if (LM_THREAD_PROVIDER_RESOLVED STREQUAL "pthread")
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(lm_thread_provider INTERFACE Threads::Threads)
endif()

message(STATUS
    "Lingvamyxa thread provider: ${LM_THREAD_PROVIDER_REQUESTED} -> ${LM_THREAD_PROVIDER_RESOLVED}"
)

# ---- External language/runtime dependencies ----

set(LM_THIRD_PARTY_TARGETS "")
set(LM_EXTERNAL_DEPS_REQUIRE_CURL FALSE)

if (LINGVAMYXA_ENABLE_EXTERNAL_DEPS)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    find_package(pcre2 CONFIG REQUIRED)
    find_package(civetweb CONFIG REQUIRED)
    find_package(yyjson CONFIG REQUIRED)
    set(LM_EXTERNAL_DEPS_REQUIRE_CURL TRUE)

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

# ---- REST/LMX server provider ----

string(TOLOWER "${LM_REST_LMX_SERVER_PROVIDER}" LM_REST_LMX_SERVER_PROVIDER_REQUESTED)
set(LM_REST_LMX_SERVER_PROVIDER_VALUES auto none civetweb)
if (NOT LM_REST_LMX_SERVER_PROVIDER_REQUESTED IN_LIST LM_REST_LMX_SERVER_PROVIDER_VALUES)
    message(FATAL_ERROR
        "Unsupported LM_REST_LMX_SERVER_PROVIDER='${LM_REST_LMX_SERVER_PROVIDER}'. "
        "Expected one of: auto, none, civetweb."
    )
endif()

if (LM_THREAD_PROVIDER_RESOLVED STREQUAL "single")
    if (LM_REST_LMX_SERVER_PROVIDER_REQUESTED STREQUAL "civetweb")
        message(FATAL_ERROR
            "LM_REST_LMX_SERVER_PROVIDER=civetweb cannot be used with "
            "LM_THREAD_PROVIDER=single because CivetWeb dispatches requests "
            "from worker threads while the single provider's mutexes are no-ops"
        )
    endif()
    set(LM_REST_LMX_SERVER_PROVIDER_RESOLVED none)
elseif (LM_REST_LMX_SERVER_PROVIDER_REQUESTED STREQUAL "none")
    set(LM_REST_LMX_SERVER_PROVIDER_RESOLVED none)
else()
    if (NOT TARGET civetweb_bundled AND NOT TARGET civetweb::civetweb)
        find_package(civetweb CONFIG QUIET)
    endif()

    if (TARGET civetweb_bundled)
        set(LM_REST_LMX_SERVER_PROVIDER_TARGET civetweb_bundled)
    elseif (TARGET civetweb::civetweb)
        set(LM_REST_LMX_SERVER_PROVIDER_TARGET civetweb::civetweb)
    endif()

    if (LM_REST_LMX_SERVER_PROVIDER_TARGET)
        set(LM_REST_LMX_SERVER_PROVIDER_RESOLVED civetweb)
    elseif (LM_REST_LMX_SERVER_PROVIDER_REQUESTED STREQUAL "civetweb")
        message(FATAL_ERROR
            "LM_REST_LMX_SERVER_PROVIDER=civetweb requires either the "
            "civetweb_bundled or civetweb::civetweb target"
        )
    else()
        set(LM_REST_LMX_SERVER_PROVIDER_RESOLVED none)
        message(STATUS
            "CivetWeb was not found; automatic REST/LMX server provider is none"
        )
    endif()
endif()

string(TOUPPER
    "${LM_REST_LMX_SERVER_PROVIDER_RESOLVED}"
    LM_REST_LMX_SERVER_PROVIDER_DEFINE_SUFFIX
)
add_library(lm_rest_lmx_server_provider INTERFACE)
target_compile_definitions(lm_rest_lmx_server_provider INTERFACE
    "LM_REST_LMX_SERVER_PROVIDER=LM_REST_LMX_SERVER_PROVIDER_${LM_REST_LMX_SERVER_PROVIDER_DEFINE_SUFFIX}"
)
if (LM_REST_LMX_SERVER_PROVIDER_RESOLVED STREQUAL "civetweb")
    target_link_libraries(lm_rest_lmx_server_provider INTERFACE
        ${LM_REST_LMX_SERVER_PROVIDER_TARGET}
    )
endif()

message(STATUS
    "Lingvamyxa REST/LMX server provider: "
    "${LM_REST_LMX_SERVER_PROVIDER_REQUESTED} -> ${LM_REST_LMX_SERVER_PROVIDER_RESOLVED}"
)

# ---- REST/LMX client provider ----

string(TOLOWER "${LM_REST_LMX_CLIENT_PROVIDER}" LM_REST_LMX_CLIENT_PROVIDER_REQUESTED)
set(LM_REST_LMX_CLIENT_PROVIDER_VALUES auto none libcurl winhttp)
if (NOT LM_REST_LMX_CLIENT_PROVIDER_REQUESTED IN_LIST LM_REST_LMX_CLIENT_PROVIDER_VALUES)
    message(FATAL_ERROR
        "Unsupported LM_REST_LMX_CLIENT_PROVIDER='${LM_REST_LMX_CLIENT_PROVIDER}'. "
        "Expected one of: auto, none, libcurl, winhttp."
    )
endif()

if (LM_REST_LMX_CLIENT_PROVIDER_REQUESTED STREQUAL "auto")
    if (WIN32)
        set(LM_REST_LMX_CLIENT_PROVIDER_RESOLVED winhttp)
    elseif (UNIX)
        set(LM_REST_LMX_CLIENT_PROVIDER_RESOLVED libcurl)
    else()
        set(LM_REST_LMX_CLIENT_PROVIDER_RESOLVED none)
        message(STATUS
            "No automatic REST/LMX client provider is known for this platform; using none"
        )
    endif()
else()
    set(LM_REST_LMX_CLIENT_PROVIDER_RESOLVED
        "${LM_REST_LMX_CLIENT_PROVIDER_REQUESTED}"
    )
endif()

if (LM_REST_LMX_CLIENT_PROVIDER_RESOLVED STREQUAL "winhttp" AND NOT WIN32)
    message(FATAL_ERROR
        "LM_REST_LMX_CLIENT_PROVIDER=winhttp requires a Windows target"
    )
endif()

set(LM_REST_LMX_CURL_REQUIRED FALSE)
if (LM_EXTERNAL_DEPS_REQUIRE_CURL OR
    LM_REST_LMX_CLIENT_PROVIDER_REQUESTED STREQUAL "libcurl")
    set(LM_REST_LMX_CURL_REQUIRED TRUE)
endif()

if (LM_EXTERNAL_DEPS_REQUIRE_CURL OR
    LM_REST_LMX_CLIENT_PROVIDER_RESOLVED STREQUAL "libcurl")
    if (LM_REST_LMX_CURL_REQUIRED)
        find_package(CURL REQUIRED)
    else()
        find_package(CURL QUIET)
    endif()
endif()

set(LM_REST_LMX_BUNDLED_CURL_SELECTED FALSE)
if (LINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY AND NOT WIN32 AND
    TARGET CURL::libcurl)
    get_target_property(
        LM_REST_LMX_CURL_ALIASED_TARGET
        CURL::libcurl
        ALIASED_TARGET
    )
    if (LM_REST_LMX_CURL_ALIASED_TARGET STREQUAL "libcurl_static" OR
        LM_REST_LMX_CURL_ALIASED_TARGET STREQUAL "libcurl")
        set(LM_REST_LMX_BUNDLED_CURL_SELECTED TRUE)
    endif()
endif()

if (LM_REST_LMX_CLIENT_PROVIDER_RESOLVED STREQUAL "libcurl" AND
    LM_REST_LMX_BUNDLED_CURL_SELECTED)
    if (LM_REST_LMX_CLIENT_PROVIDER_REQUESTED STREQUAL "auto")
        set(LM_REST_LMX_CLIENT_PROVIDER_RESOLVED none)
        message(STATUS
            "Bundled CURL has no TLS backend on this platform; "
            "automatic REST/LMX client provider is none"
        )
    else()
        message(FATAL_ERROR
            "LM_REST_LMX_CLIENT_PROVIDER=libcurl cannot use bundled CURL "
            "without a TLS backend on this platform"
        )
    endif()
endif()

if (LM_REST_LMX_CLIENT_PROVIDER_RESOLVED STREQUAL "libcurl" AND
    NOT TARGET CURL::libcurl)
    if (LM_REST_LMX_CLIENT_PROVIDER_REQUESTED STREQUAL "auto")
        set(LM_REST_LMX_CLIENT_PROVIDER_RESOLVED none)
        message(STATUS "CURL was not found; automatic REST/LMX client provider is none")
    else()
        message(FATAL_ERROR
            "LM_REST_LMX_CLIENT_PROVIDER=libcurl requires the CURL::libcurl target"
        )
    endif()
endif()

if (LM_EXTERNAL_DEPS_REQUIRE_CURL AND NOT TARGET CURL::libcurl)
    message(FATAL_ERROR
        "LINGVAMYXA_ENABLE_EXTERNAL_DEPS requires the CURL::libcurl target"
    )
endif()

string(TOUPPER
    "${LM_REST_LMX_CLIENT_PROVIDER_RESOLVED}"
    LM_REST_LMX_CLIENT_PROVIDER_DEFINE_SUFFIX
)
add_library(lm_rest_lmx_client_provider INTERFACE)
target_compile_definitions(lm_rest_lmx_client_provider INTERFACE
    "LM_REST_LMX_CLIENT_PROVIDER=LM_REST_LMX_CLIENT_PROVIDER_${LM_REST_LMX_CLIENT_PROVIDER_DEFINE_SUFFIX}"
)

if (LM_REST_LMX_CLIENT_PROVIDER_RESOLVED STREQUAL "libcurl")
    target_link_libraries(lm_rest_lmx_client_provider INTERFACE CURL::libcurl)
elseif (LM_REST_LMX_CLIENT_PROVIDER_RESOLVED STREQUAL "winhttp")
    target_link_libraries(lm_rest_lmx_client_provider INTERFACE winhttp)
endif()

message(STATUS
    "Lingvamyxa REST/LMX client provider: "
    "${LM_REST_LMX_CLIENT_PROVIDER_REQUESTED} -> ${LM_REST_LMX_CLIENT_PROVIDER_RESOLVED}"
)

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
set(LM1_GENERATED_REST_LMX_HTTP_CLIENT_C
    "${LM1_GENERATED_DIR}/rest_lmx_http_client.lm1.c"
)
set(LM1_GENERATED_REST_LMX_HTTP_SERVER_C
    "${LM1_GENERATED_DIR}/rest_lmx_http_server.lm1.c"
)
set(LM2_CORE_REGISTRY "${CMAKE_CURRENT_SOURCE_DIR}/lm2/core.lm2")
set(LM2_TRANS_REGISTRY "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans_registry.lm2")
set(LM2_PARSER_REGISTRY "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser_registry.lm2")
set(LM2_PARSER_TEXT "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser_text.lm2")
set(LM2_PARSER_REGISTRY_SELFTEST "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser_registry_selftest.lm2")
set(LM2_OWN_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/own_abi.lm2")
set(LM2_PARSER_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/parser_abi.lm2")
set(LM2_REGISTRY_SOURCE_LOADER_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/registry_source_loader_abi.lm2")
set(LM2_TRANS_INTERNAL_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans_internal_abi.lm2")
set(LM2_LIBC_ABI "${CMAKE_CURRENT_SOURCE_DIR}/lm2/libc_abi.lm2")
set(LM2_REGISTRY_SOURCE_LOADER_UNIT_PAYLOAD "${CMAKE_CURRENT_SOURCE_DIR}/lm2/registry_source_loader_unit_payload.lm2")
set(LM2_TABLE_DESCRIPTOR "${CMAKE_CURRENT_SOURCE_DIR}/lm2/table_descriptor.lm2")
set(LM2_TABLE_DESCRIPTOR_MODEL "${CMAKE_CURRENT_SOURCE_DIR}/lm2/table_descriptor_model.lm2")
set(LM2_REST_LMX_HTTP_CLIENT
    "${CMAKE_CURRENT_SOURCE_DIR}/lm2/rest_lmx_http_client.lm2"
)
set(LM2_REST_LMX_HTTP_CLIENT_ABI
    "${CMAKE_CURRENT_SOURCE_DIR}/lm2/rest_lmx_http_client_abi.lm2"
)
set(LM2_REST_LMX_HTTP_SERVER
    "${CMAKE_CURRENT_SOURCE_DIR}/lm2/rest_lmx_http_server.lm2"
)
set(LM2_REST_LMX_HTTP_SERVER_ABI
    "${CMAKE_CURRENT_SOURCE_DIR}/lm2/rest_lmx_http_server_abi.lm2"
)
set(LM2_TO_LM1_REGISTRY_DEPS
    "${LM2_CORE_REGISTRY}"
    "${LM2_TRANS_REGISTRY}"
    "${LM2_PARSER_REGISTRY}"
    "${LM2_PARSER_TEXT}"
    "${LM2_PARSER_REGISTRY_SELFTEST}"
    "${LM2_OWN_ABI}"
    "${LM2_PARSER_ABI}"
    "${LM2_REGISTRY_SOURCE_LOADER_ABI}"
    "${LM2_TRANS_INTERNAL_ABI}"
    "${LM2_LIBC_ABI}"
    "${LM2_REGISTRY_SOURCE_LOADER_UNIT_PAYLOAD}"
    "${LM2_TABLE_DESCRIPTOR}"
    "${LM2_TABLE_DESCRIPTOR_MODEL}"
    "${LM2_REST_LMX_HTTP_CLIENT_ABI}"
    "${LM2_REST_LMX_HTTP_SERVER_ABI}"
)

# ---- Generated L1 ownership helper ----

if (BUILD_TRANS_LM0 OR BUILD_PRINT_TREE_LM0 OR BUILD_VCPKG_FETCH_LM0 OR BUILD_BUILD_CORE_LM0 OR BUILD_MAKE_LM0)
    add_library(own.lm1 STATIC
        "${LM1_GENERATED_OWN_C}"
    )

    set_target_properties(own.lm1 PROPERTIES
        OUTPUT_NAME "own.lm0"
    )

    target_include_directories(own.lm1 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1
    )

    target_link_libraries(own.lm1 PUBLIC
        lm_thread_provider
    )

    if (MSVC)
        target_compile_options(own.lm1 PRIVATE /W4)
    else()
        target_compile_options(own.lm1 PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# ---- Optional generated REST/LMX HTTP server adapter ----

if (TARGET own.lm1)
    set_source_files_properties(
        "${LM1_GENERATED_REST_LMX_HTTP_SERVER_C}"
        PROPERTIES GENERATED TRUE
    )
    add_library(rest_lmx_http_server.lm1 STATIC EXCLUDE_FROM_ALL
        "${LM1_GENERATED_REST_LMX_HTTP_SERVER_C}"
    )

    target_include_directories(rest_lmx_http_server.lm1 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1
    )

    target_link_libraries(rest_lmx_http_server.lm1 PUBLIC
        own.lm1
        lm_rest_lmx_server_provider
    )

    if (MSVC)
        target_compile_options(rest_lmx_http_server.lm1 PRIVATE /W4)
    else()
        target_compile_options(rest_lmx_http_server.lm1 PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_library(lm_rest_lmx_default_server INTERFACE)
    target_link_libraries(lm_rest_lmx_default_server INTERFACE
        rest_lmx_http_server.lm1
    )
    target_compile_definitions(lm_rest_lmx_default_server INTERFACE
        LM_REST_LMX_INSTALL_DEFAULT_SERVER=1
    )
endif()

# ---- Optional generated REST/LMX HTTP client adapter ----

if (TARGET own.lm1)
    set_source_files_properties(
        "${LM1_GENERATED_REST_LMX_HTTP_CLIENT_C}"
        PROPERTIES GENERATED TRUE
    )
    add_library(rest_lmx_http_client.lm1 STATIC EXCLUDE_FROM_ALL
        "${LM1_GENERATED_REST_LMX_HTTP_CLIENT_C}"
    )

    target_include_directories(rest_lmx_http_client.lm1 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lm1
    )

    target_link_libraries(rest_lmx_http_client.lm1 PUBLIC
        own.lm1
        lm_rest_lmx_client_provider
    )

    if (MSVC)
        target_compile_options(rest_lmx_http_client.lm1 PRIVATE /W4)
    else()
        target_compile_options(rest_lmx_http_client.lm1 PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_library(lm_rest_lmx_default_client INTERFACE)
    target_link_libraries(lm_rest_lmx_default_client INTERFACE
        rest_lmx_http_client.lm1
    )
    target_compile_definitions(lm_rest_lmx_default_client INTERFACE
        LM_REST_LMX_INSTALL_DEFAULT_CLIENT=1
    )
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
            "${CMAKE_CURRENT_SOURCE_DIR}/lm2/trans_library.lm2"
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

    add_custom_target(refresh_rest_lmx_http_client_lm1
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${LM2_REST_LMX_HTTP_CLIENT}"
            "${LM1_GENERATED_REST_LMX_HTTP_CLIENT_C}"
        DEPENDS
            trans.lm0
            "${LM2_REST_LMX_HTTP_CLIENT}"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Refreshing lm1/build/rest_lmx_http_client.lm1.c from lm2/rest_lmx_http_client.lm2"
        VERBATIM
    )
    if (TARGET rest_lmx_http_client.lm1)
        add_dependencies(
            rest_lmx_http_client.lm1
            refresh_rest_lmx_http_client_lm1
        )
    endif()

    add_custom_target(refresh_rest_lmx_http_server_lm1
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LM1_GENERATED_DIR}"
        COMMAND $<TARGET_FILE:trans.lm0>
            "${LM2_REST_LMX_HTTP_SERVER}"
            "${LM1_GENERATED_REST_LMX_HTTP_SERVER_C}"
        DEPENDS
            trans.lm0
            "${LM2_REST_LMX_HTTP_SERVER}"
            ${LM2_TO_LM1_REGISTRY_DEPS}
        COMMENT "Refreshing lm1/build/rest_lmx_http_server.lm1.c from lm2/rest_lmx_http_server.lm2"
        VERBATIM
    )
    if (TARGET rest_lmx_http_server.lm1)
        add_dependencies(
            rest_lmx_http_server.lm1
            refresh_rest_lmx_http_server_lm1
        )
    endif()

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
            refresh_rest_lmx_http_client_lm1
            refresh_rest_lmx_http_server_lm1
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

    target_link_libraries(make.lm0 PRIVATE
        own.lm1
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
            "LM_THREAD_PROVIDER=${LM_THREAD_PROVIDER_RESOLVED}"
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
