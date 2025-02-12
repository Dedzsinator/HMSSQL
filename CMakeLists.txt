# Main CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(BusTub VERSION 2022.8 DESCRIPTION "The BusTub Relational Database Management System (Educational) @ https://github.com/cmu-db/bustub" LANGUAGES C CXX)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to `Debug` as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

if (EMSCRIPTEN)
    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)
endif ()

# People keep running CMake in the wrong folder, completely nuking their project or creating weird bugs.
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" PATH_TO_CMAKELISTS_TXT)
if (EXISTS "${PATH_TO_CMAKELISTS_TXT}")
    message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif ()

# Expected directory structure.
set(BUSTUB_BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/build_support")
set(BUSTUB_CLANG_SEARCH_PATH "/usr/local/bin" "/usr/bin" "/usr/local/opt/llvm/bin" "/usr/local/opt/llvm@12/bin" "/usr/local/Cellar/llvm/12.0.1/bin" "/opt/homebrew/opt/llvm@12/bin/")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION MATCHES "^12.")
        message(STATUS "You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    else ()
        message(WARNING "!! We recommend that you use clang-12 for developing BusTub. You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, a different version.")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(STATUS "You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
else ()
    message(WARNING "!! We recommend that you use clang-12 for developing BusTub. You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, which is not clang.")
endif ()

######################################################################################################################
# DEPENDENCIES
######################################################################################################################

# CTest
enable_testing()

# Remove clang-format and clang-tidy references

# Compiler flags.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused-parameter -Wno-attributes") #TODO: remove
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

# Output directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Includes.
set(BUSTUB_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)
set(BUSTUB_TEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test/include)
set(BUSTUB_THIRD_PARTY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party ${PROJECT_SOURCE_DIR}/third_party/fmt/include ${PROJECT_SOURCE_DIR}/third_party/libpg_query/include ${PROJECT_SOURCE_DIR}/third_party/argparse/include)

include_directories(${BUSTUB_SRC_INCLUDE_DIR} ${BUSTUB_TEST_INCLUDE_DIR} ${BUSTUB_THIRD_PARTY_INCLUDE_DIR})
include_directories(BEFORE src) # This is needed for gtest.

function(disable_target_warnings NAME)
    target_compile_options(${NAME} PRIVATE "-w")
endfunction()

######################################################################################################################
# Other CMake modules
# MUST BE ADDED AFTER CONFIGURING COMPILER PARAMETERS
######################################################################################################################

add_subdirectory(src)
add_subdirectory(third_party)