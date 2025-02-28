cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(HMSSQL VERSION 2022.8 DESCRIPTION "The HMSSQL Relational Database Management System (Educational) @ https://github.com/Dedzsinator/HMSSQL" LANGUAGES C CXX)

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

message(STATUS "Using compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

######################################################################################################################
# DEPENDENCIES
######################################################################################################################

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
set(HMSSQL_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(HMSSQL_THIRD_PARTY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party ${PROJECT_SOURCE_DIR}/third_party/fmt/include ${PROJECT_SOURCE_DIR}/third_party/libpg_query/include ${PROJECT_SOURCE_DIR}/third_party/argparse/include ${PROJECT_SOURCE_DIR}/third_party/spdlog/include)

include_directories(${HMSSQL_SRC_INCLUDE_DIR} ${HMSSQL_THIRD_PARTY_INCLUDE_DIR})

function(disable_target_warnings NAME)
    target_compile_options(${NAME} PRIVATE "-w")
endfunction()

######################################################################################################################
# Other CMake modules
# MUST BE ADDED AFTER CONFIGURING COMPILER PARAMETERS
######################################################################################################################

add_subdirectory(src)
add_subdirectory(third_party)

# Add executables
# Update executables to not explicitly link spdlog
add_executable(hmssql_main tools/shell/shell.cpp)
target_link_libraries(hmssql_main PRIVATE 
    hmssql 
    hmssql_linenoise 
    argparse 
    utf8proc
)

add_executable(daemon tools/daemon/daemon.cpp)
target_link_libraries(daemon PRIVATE 
    hmssql 
    utf8proc
)