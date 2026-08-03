if (NOT DEFINED LM_THIRD_PARTY_SOURCE_DIR)
    set(LM_THIRD_PARTY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
endif()
get_filename_component(LM_THIRD_PARTY_SOURCE_DIR "${LM_THIRD_PARTY_SOURCE_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

function(lm_set_archive_output target output_name)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${LM_LIBS_OUTPUT_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${LM_LIBS_OUTPUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${LM_LIBS_OUTPUT_DIR}"
            OUTPUT_NAME "${output_name}"
            PREFIX "lib"
            DEBUG_POSTFIX ""
            RELEASE_POSTFIX ""
            RELWITHDEBINFO_POSTFIX ""
            MINSIZEREL_POSTFIX ""
        )
    endif()
endfunction()

function(lm_path_to_shell out_var input_path)
    file(TO_CMAKE_PATH "${input_path}" normalized_path)
    set(${out_var} "${normalized_path}" PARENT_SCOPE)
endfunction()

function(lm_path_to_sh out_var input_path)
    file(TO_CMAKE_PATH "${input_path}" normalized_path)
    if (WIN32 AND normalized_path MATCHES "^([A-Za-z]):/(.*)$")
        string(TOLOWER "${CMAKE_MATCH_1}" drive)
        set(${out_var} "/${drive}/${CMAKE_MATCH_2}" PARENT_SCOPE)
    else()
        set(${out_var} "${normalized_path}" PARENT_SCOPE)
    endif()
endfunction()

function(lm_find_posix_build_tools)
    if (WIN32)
        set(msys2_roots "")
        if (DEFINED ENV{VCPKG_ROOT})
            list(APPEND msys2_roots "$ENV{VCPKG_ROOT}/downloads/tools/msys2")
        endif()
        list(APPEND msys2_roots "C:/vcpkg/downloads/tools/msys2")

        set(bash_candidates "")
        foreach(msys2_root IN LISTS msys2_roots)
            file(GLOB root_bash_candidates
                "${msys2_root}/*/usr/bin/bash.exe"
            )
            list(APPEND bash_candidates ${root_bash_candidates})
        endforeach()
        list(REMOVE_DUPLICATES bash_candidates)
        list(SORT bash_candidates)

        set(found_bash "")
        set(found_make "")
        foreach(candidate IN LISTS bash_candidates)
            get_filename_component(candidate_dir "${candidate}" DIRECTORY)
            if (EXISTS "${candidate_dir}/make.exe")
                set(found_bash "${candidate}")
                set(found_make "${candidate_dir}/make.exe")
            endif()
        endforeach()

        if (found_bash)
            set(LM_POSIX_SH "${found_bash}" PARENT_SCOPE)
            set(LM_POSIX_MAKE "${found_make}" PARENT_SCOPE)
            return()
        endif()
    endif()

    find_program(found_sh NAMES sh bash)
    find_program(found_make NAMES make gmake)

    set(LM_POSIX_SH "${found_sh}" PARENT_SCOPE)
    set(LM_POSIX_MAKE "${found_make}" PARENT_SCOPE)
endfunction()

function(lm_add_autotools_static_library target source_dir library_name)
    cmake_parse_arguments(ARG "" "" "CONFIGURE_ARGS;DEPENDS" ${ARGN})

    lm_find_posix_build_tools()
    if (NOT LM_POSIX_SH OR NOT LM_POSIX_MAKE)
        message(FATAL_ERROR "${target} needs a POSIX shell and make. Run the vcpkg source download once or install MSYS2/Git build tools.")
    endif()

    set(build_dir "${CMAKE_CURRENT_BINARY_DIR}/third_party/${target}")
    set(prefix_dir "${LM_LIBS_OUTPUT_DIR}/local")
    set(prefix_lib "${prefix_dir}/lib/${library_name}")
    set(output_lib "${LM_LIBS_OUTPUT_DIR}/${library_name}")

    file(RELATIVE_PATH source_dir_from_build "${build_dir}" "${source_dir}")
    if (source_dir_from_build MATCHES "^[A-Za-z]:")
        lm_path_to_shell(source_dir_sh "${source_dir}")
    else()
        lm_path_to_shell(source_dir_sh "${source_dir_from_build}")
    endif()
    lm_path_to_shell(build_dir_sh "${build_dir}")
    lm_path_to_shell(prefix_dir_sh "${prefix_dir}")
    lm_path_to_shell(msys_make_sh "${LM_POSIX_MAKE}")
    lm_path_to_shell(cc_sh "${CMAKE_C_COMPILER}")
    lm_path_to_shell(ar_sh "${CMAKE_AR}")
    lm_path_to_shell(ranlib_sh "${CMAKE_RANLIB}")
    get_filename_component(posix_bin "${LM_POSIX_SH}" DIRECTORY)
    lm_path_to_sh(posix_bin_sh "${posix_bin}")

    set(configure_line
        "export PATH='${posix_bin_sh}':$PATH && cd '${build_dir_sh}' && '${source_dir_sh}/configure' --disable-shared --enable-static --prefix='${prefix_dir_sh}' CC='${cc_sh}' AR='${ar_sh}' RANLIB='${ranlib_sh}' ${ARG_CONFIGURE_ARGS} && '${msys_make_sh}' -j${LM_AUTOTOOLS_JOBS} && '${msys_make_sh}' install"
    )

    add_custom_command(
        OUTPUT "${output_lib}"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${build_dir}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${build_dir}"
        COMMAND "${LM_POSIX_SH}" -lc "${configure_line}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${prefix_lib}" "${output_lib}"
        DEPENDS ${ARG_DEPENDS}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
    )

    add_custom_target(${target} DEPENDS "${output_lib}")
endfunction()

function(lm_register_bundled_third_party out_var)
    set(local_targets "")

    # decimal -> decNumber
    set(DECNUMBER_DIR "${LM_THIRD_PARTY_SOURCE_DIR}/decNumber/decNumber-icu-368")
    add_library(decnumber STATIC
        "${DECNUMBER_DIR}/decContext.c"
        "${DECNUMBER_DIR}/decNumber.c"
    )
    lm_set_archive_output(decnumber "decnumber")
    target_include_directories(decnumber PUBLIC "${DECNUMBER_DIR}")
    target_compile_features(decnumber PUBLIC c_std_99)
    target_compile_definitions(decnumber PUBLIC DECNUMDIGITS=64)
    list(APPEND local_targets decnumber)

    # zlib -> libz.a
    set(ZLIB_DIR "${LM_THIRD_PARTY_SOURCE_DIR}/zlib")
    add_library(zlib_bundled STATIC
        "${ZLIB_DIR}/adler32.c"
        "${ZLIB_DIR}/compress.c"
        "${ZLIB_DIR}/crc32.c"
        "${ZLIB_DIR}/deflate.c"
        "${ZLIB_DIR}/gzclose.c"
        "${ZLIB_DIR}/gzlib.c"
        "${ZLIB_DIR}/gzread.c"
        "${ZLIB_DIR}/gzwrite.c"
        "${ZLIB_DIR}/infback.c"
        "${ZLIB_DIR}/inffast.c"
        "${ZLIB_DIR}/inflate.c"
        "${ZLIB_DIR}/inftrees.c"
        "${ZLIB_DIR}/trees.c"
        "${ZLIB_DIR}/uncompr.c"
        "${ZLIB_DIR}/zutil.c"
    )
    lm_set_archive_output(zlib_bundled "z")
    target_include_directories(zlib_bundled PUBLIC "${ZLIB_DIR}")
    target_compile_features(zlib_bundled PUBLIC c_std_99)
    list(APPEND local_targets zlib_bundled)

    # SQL -> SQLite amalgamation
    set(SQLITE_DIR "${LM_THIRD_PARTY_SOURCE_DIR}/sqlite")
    add_library(sqlite3_bundled STATIC "${SQLITE_DIR}/sqlite3.c")
    lm_set_archive_output(sqlite3_bundled "sqlite3")
    target_include_directories(sqlite3_bundled PUBLIC "${SQLITE_DIR}")
    target_compile_features(sqlite3_bundled PUBLIC c_std_99)
    target_compile_definitions(sqlite3_bundled
        PUBLIC
            SQLITE_ENABLE_JSON1=1
            SQLITE_THREADSAFE=1
    )
    if (WIN32)
        target_compile_definitions(sqlite3_bundled PRIVATE SQLITE_OS_WIN=1)
    endif()
    list(APPEND local_targets sqlite3_bundled)

    # JSON -> yyjson
    set(YYJSON_DIR "${LM_THIRD_PARTY_SOURCE_DIR}/yyjson")
    add_library(yyjson_bundled STATIC "${YYJSON_DIR}/src/yyjson.c")
    lm_set_archive_output(yyjson_bundled "yyjson")
    target_include_directories(yyjson_bundled PUBLIC "${YYJSON_DIR}/src")
    target_compile_features(yyjson_bundled PUBLIC c_std_99)
    list(APPEND local_targets yyjson_bundled)

    # HTTP server facade -> CivetWeb, intentionally without SSL at this layer.
    set(CIVETWEB_DIR "${LM_THIRD_PARTY_SOURCE_DIR}/citeweb")
    add_library(civetweb_bundled STATIC "${CIVETWEB_DIR}/src/civetweb.c")
    lm_set_archive_output(civetweb_bundled "civetweb")
    target_include_directories(civetweb_bundled
        PUBLIC
            "${CIVETWEB_DIR}/include"
            "${CIVETWEB_DIR}/src"
    )
    target_compile_features(civetweb_bundled PUBLIC c_std_99)
    target_compile_definitions(civetweb_bundled
        PRIVATE
            NO_SSL
            NO_FILES
            NO_CGI
            NO_CACHING
            USE_STACK_SIZE=0
    )
    if (WIN32)
        target_link_libraries(civetweb_bundled PUBLIC ws2_32)
    endif()
    list(APPEND local_targets civetweb_bundled)

    # Crypto/auth -> libsodium
    set(LIBSODIUM_DIR "${LM_THIRD_PARTY_SOURCE_DIR}/libsodium")
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
    add_library(libsodium_vendor STATIC ${LIBSODIUM_SOURCES})
    lm_set_archive_output(libsodium_vendor "sodium")
    target_include_directories(libsodium_vendor
        PUBLIC
            "${CMAKE_CURRENT_BINARY_DIR}/generated/libsodium"
            "${LIBSODIUM_DIR}/src/libsodium/include"
        PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/generated/libsodium/sodium"
            "${LIBSODIUM_DIR}/src/libsodium/include/sodium"
    )
    target_compile_features(libsodium_vendor PUBLIC c_std_99)
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
        target_link_libraries(libsodium_vendor INTERFACE advapi32)
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
    list(APPEND local_targets libsodium_vendor)

    # Regex -> PCRE2 8/16/32-bit code-unit libraries + JIT.
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_PCRE2_8 ON CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_PCRE2_16 ON CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_PCRE2_32 ON CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_PCRE2GREP OFF CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(PCRE2_SUPPORT_JIT ON CACHE BOOL "" FORCE)
    set(PCRE2_SUPPORT_UNICODE ON CACHE BOOL "" FORCE)
    set(PCRE2_SUPPORT_LIBBZ2 OFF CACHE BOOL "" FORCE)
    set(PCRE2_SUPPORT_LIBZ OFF CACHE BOOL "" FORCE)
    set(PCRE2_SUPPORT_LIBEDIT OFF CACHE BOOL "" FORCE)
    set(PCRE2_SUPPORT_LIBREADLINE OFF CACHE BOOL "" FORCE)
    set(PCRE2_SHOW_REPORT OFF CACHE BOOL "" FORCE)
    set(PCRE2_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/pcre2-src")
    file(REMOVE_RECURSE "${PCRE2_SOURCE_DIR}")
    file(COPY "${LM_THIRD_PARTY_SOURCE_DIR}/pcre2/" DESTINATION "${PCRE2_SOURCE_DIR}")
    file(REMOVE_RECURSE "${PCRE2_SOURCE_DIR}/deps/sljit")
    file(COPY "${LM_THIRD_PARTY_SOURCE_DIR}/sljit/" DESTINATION "${PCRE2_SOURCE_DIR}/deps/sljit")
    add_subdirectory(
        "${PCRE2_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/third_party/pcre2"
        EXCLUDE_FROM_ALL
    )
    lm_set_archive_output(pcre2-8-static "pcre2-8")
    lm_set_archive_output(pcre2-16-static "pcre2-16")
    lm_set_archive_output(pcre2-32-static "pcre2-32")
    lm_set_archive_output(pcre2-posix-static "pcre2-posix")
    list(APPEND local_targets
        pcre2-8-static
        pcre2-16-static
        pcre2-32-static
        pcre2-posix-static
    )

    # Network client -> libcurl. On Windows, use native Schannel/SSPI instead of OpenSSL.
    set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    set(BUILD_STATIC_CURL OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "" FORCE)
    set(BUILD_MISC_DOCS OFF CACHE BOOL "" FORCE)
    set(ENABLE_CURL_MANUAL OFF CACHE BOOL "" FORCE)
    set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
    set(CURL_USE_CMAKECONFIG OFF CACHE BOOL "" FORCE)
    set(CURL_USE_PKGCONFIG OFF CACHE BOOL "" FORCE)
    set(CURL_ZLIB OFF CACHE BOOL "" FORCE)
    set(CURL_BROTLI OFF CACHE BOOL "" FORCE)
    set(CURL_ZSTD OFF CACHE BOOL "" FORCE)
    set(USE_NGHTTP2 OFF CACHE BOOL "" FORCE)
    set(USE_LIBIDN2 OFF CACHE BOOL "" FORCE)
    set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
    set(CURL_USE_LIBSSH2 OFF CACHE BOOL "" FORCE)
    set(CURL_USE_LIBSSH OFF CACHE BOOL "" FORCE)
    set(CURL_USE_GSASL OFF CACHE BOOL "" FORCE)
    set(CURL_USE_GSSAPI OFF CACHE BOOL "" FORCE)
    set(HTTP_ONLY ON CACHE BOOL "" FORCE)
    if (WIN32)
        set(CURL_ENABLE_SSL ON CACHE BOOL "" FORCE)
        set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
        set(CURL_WINDOWS_SSPI ON CACHE BOOL "" FORCE)
    else()
        set(CURL_ENABLE_SSL OFF CACHE BOOL "" FORCE)
    endif()
    add_subdirectory(
        "${LM_THIRD_PARTY_SOURCE_DIR}/curl"
        "${CMAKE_CURRENT_BINARY_DIR}/third_party/curl"
        EXCLUDE_FROM_ALL
    )
    if (TARGET libcurl_static)
        lm_set_archive_output(libcurl_static "curl")
        list(APPEND local_targets libcurl_static)
    elseif(TARGET libcurl)
        lm_set_archive_output(libcurl "curl")
        list(APPEND local_targets libcurl)
    endif()

    # bigint/real -> GMP/MPFR autotools snapshots.
    set(GMP_DIR "${LM_THIRD_PARTY_SOURCE_DIR}/gmp")
    lm_add_autotools_static_library(
        gmp_bundled
        "${GMP_DIR}"
        "libgmp.a"
        CONFIGURE_ARGS "--enable-assembly=no --with-readline=no"
    )
    list(APPEND local_targets gmp_bundled)

    set(MPFR_DIR "${LM_THIRD_PARTY_SOURCE_DIR}/mpfr")
    lm_path_to_shell(mpfr_gmp_prefix_sh "${LM_LIBS_OUTPUT_DIR}/local")
    lm_add_autotools_static_library(
        mpfr_bundled
        "${MPFR_DIR}"
        "libmpfr.a"
        CONFIGURE_ARGS "--with-gmp='${mpfr_gmp_prefix_sh}'"
        DEPENDS gmp_bundled
    )
    list(APPEND local_targets mpfr_bundled)

    set(${out_var} ${local_targets} PARENT_SCOPE)
endfunction()
