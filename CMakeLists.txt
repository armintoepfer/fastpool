###############################################
# CMake build script for the THREADPOOL library
###############################################

cmake_policy(SET CMP0048 NEW)
project(TP VERSION 1.0.0 LANGUAGES CXX C)
cmake_minimum_required(VERSION 3.2)

set(ROOT_PROJECT_NAME ${PROJECT_NAME} CACHE STRING "root project name")

# Build type
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release Profile RelWithDebInfo ReleaseWithAssert" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Build-time options
option(TP_build_bin    "Build binaries." ON)

# Main project paths
set(TP_RootDir       ${TP_SOURCE_DIR})
set(TP_IncludeDir    ${TP_RootDir}/include)
set(TP_ExampleDir    ${TP_RootDir}/example)
set(TP_ThirdParty    ${TP_RootDir}/third-party)

# Project configuration
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})

# Threads
if (NOT Threads)
    find_package(Threads REQUIRED)
endif()

include(CheckCXXCompilerFlag)

add_library(threadpool INTERFACE)
target_include_directories(threadpool INTERFACE
    ${TP_IncludeDir}
    ${TP_ThirdParty}/concurrentqueue)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(threadpool rt)
ENDIF()
target_link_libraries(threadpool
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)

# Example
add_subdirectory(${TP_ExampleDir})