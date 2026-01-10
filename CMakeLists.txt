cmake_minimum_required(VERSION 3.14)
project(SystemC_SoC VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF)

#####                   Options                      #####
option(USE_CONAN "Whether to use Conan." OFF)
option(USE_VCPKG "Whether to use VCPKG." OFF)
option(USE_CPM "Whether to use CPM." ON)

option(ENABLE_COVERAGE "Enable code coverage reoporting" OFF)
option(ENABLE_SANITIZER "Enable Address and Undefined Sanitizers" OFF)
option(ENABLE_SANITIZE_ADDR "Enable address sanitize." OFF)
option(ENABLE_SANITIZE_UNDEF "Enable undefined sanitize." OFF)
option(ENABLE_SANITIZE_LEAK "Enable leak sanitize (Gcc/Clang only)." OFF)
option(ENABLE_SANITIZE_THREAD "Enable thread sanitize (Gcc/Clang only)." OFF)
option(ENABLE_CMAKE_FORMAT "Enable to add cmake-format." OFF)


#####                   Cmake Module                  #####
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(AddGitSubmodule)
#include(Docs)
#include(Tools)

find_package(SystemC CONFIG REQUIRED)

include (FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.12.0
)

message(STATUS "Fetching SystemC from GitHub...")
FetchContent_Declare(
  SystemC
  GIT_REPOSITORY https://github.com/accellera-official/systemc.git
  GIT_TAG        2.3.4  # Phiên bản ổn định phổ biến
)

set(ENABLE_PHASE_CALLBACKS OFF CACHE BOOL "" FORCE) 
set(ENABLE_ASSERTIONS ON CACHE BOOL "" FORCE)      
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)          

FetchContent_MakeAvailable(SystemC)
FetchContent_MakeAvailable(Catch2)

if(ENABLE_SANITIZER)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_subdirectory(src)
add_subdirectory(testbench)