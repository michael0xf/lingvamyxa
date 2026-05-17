cmake_minimum_required(VERSION 3.22)

get_filename_component(LM_EXTRACT_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if (NOT DEFINED LM_THIRD_PARTY_ARCHIVE_DIR)
    set(LM_THIRD_PARTY_ARCHIVE_DIR "${LM_EXTRACT_PROJECT_ROOT}/third_party")
endif()

if (NOT DEFINED LM_THIRD_PARTY_EXTRACT_DIR)
    set(LM_THIRD_PARTY_EXTRACT_DIR "${LM_EXTRACT_PROJECT_ROOT}/build/third_party")
endif()

if (NOT DEFINED LM_THIRD_PARTY_EXTRACT_DRY_RUN)
    set(LM_THIRD_PARTY_EXTRACT_DRY_RUN OFF)
endif()

file(TO_CMAKE_PATH "${LM_THIRD_PARTY_ARCHIVE_DIR}" LM_THIRD_PARTY_ARCHIVE_DIR)
file(TO_CMAKE_PATH "${LM_THIRD_PARTY_EXTRACT_DIR}" LM_THIRD_PARTY_EXTRACT_DIR)

if (NOT IS_DIRECTORY "${LM_THIRD_PARTY_ARCHIVE_DIR}")
    message(FATAL_ERROR "third_party archive directory was not found: ${LM_THIRD_PARTY_ARCHIVE_DIR}")
endif()

if (NOT LM_THIRD_PARTY_EXTRACT_DRY_RUN)
    file(MAKE_DIRECTORY "${LM_THIRD_PARTY_EXTRACT_DIR}")
endif()

file(GLOB LM_THIRD_PARTY_ZIP_FILES LIST_DIRECTORIES false "${LM_THIRD_PARTY_ARCHIVE_DIR}/*.zip")
list(SORT LM_THIRD_PARTY_ZIP_FILES)

if (NOT LM_THIRD_PARTY_ZIP_FILES)
    message(STATUS "No third_party zip archives found in ${LM_THIRD_PARTY_ARCHIVE_DIR}")
    return()
endif()

foreach(archive_path IN LISTS LM_THIRD_PARTY_ZIP_FILES)
    get_filename_component(archive_name "${archive_path}" NAME)

    message(STATUS "Extracting ${archive_name} into ${LM_THIRD_PARTY_EXTRACT_DIR}")

    if (LM_THIRD_PARTY_EXTRACT_DRY_RUN)
        continue()
    endif()

    file(ARCHIVE_EXTRACT
        INPUT "${archive_path}"
        DESTINATION "${LM_THIRD_PARTY_EXTRACT_DIR}"
    )
endforeach()
