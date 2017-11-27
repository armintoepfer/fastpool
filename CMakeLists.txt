#############################################
# CMake build script for the FASFPOOL library
#############################################

cmake_policy(SET CMP0048 NEW)
project(FP VERSION 1.0.0 LANGUAGES CXX C)
cmake_minimum_required(VERSION 3.2)

set(ROOT_PROJECT_NAME ${PROJECT_NAME} CACHE STRING "root project name")

# Build type
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release Profile RelWithDebInfo ReleaseWithAssert" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Build-time options
option(FP_build_bin    "Build binaries." ON)

# Main project paths
set(FP_RootDir       ${FP_SOURCE_DIR})
set(FP_IncludeDir    ${FP_RootDir}/include)
set(FP_ExampleDir    ${FP_RootDir}/example)
set(FP_ThirdParty    ${FP_RootDir}/third-party)

# Project configuration
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})

# Threads
if (NOT Threads)
    find_package(Threads REQUIRED)
endif()

include(CheckCXXCompilerFlag)

add_library(fastpool INTERFACE)
target_include_directories(fastpool INTERFACE
    ${FP_IncludeDir}
    ${FP_ThirdParty}/concurrentqueue)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(fastpool INTERFACE rt)
ENDIF()
target_link_libraries(fastpool INTERFACE
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)

# Example
if (FP_build_bin)
    add_subdirectory(${FP_ExampleDir})
endif()