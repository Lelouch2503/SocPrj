cmake_minimum_required(VERSION 3.24...3.30) # Upgrade to 3.24 to use OVERRIDE features
# For CMake 4.x compatibility with older dependencies
if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
endif()
project(SystemC_SoC VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BUILD_SHARED_LIBS OFF)

#####                   Options                      #####
option(USE_CONAN "Whether to use Conan." OFF)
option(USE_VCPKG "Whether to use VCPKG." OFF)
option(USE_CPM "Whether to use CPM." ON)

option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(ENABLE_SANITIZER "Enable Address and Undefined Sanitizers" OFF)
option(ENABLE_SANITIZE_ADDR "Enable address sanitizer." OFF)
option(ENABLE_SANITIZE_UNDEF "Enable undefined behavior sanitizer." OFF)
option(ENABLE_SANITIZE_LEAK "Enable leak sanitizer (GCC/Clang only)." OFF)
option(ENABLE_SANITIZE_THREAD "Enable thread sanitizer (GCC/Clang only)." OFF)
option(ENABLE_CMAKE_FORMAT "Enable cmake-format integration." OFF)

#####                   CMake Modules                #####
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(AddGitSubmodule)
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

#####                   Dependencies                  #####
message(STATUS "Configuring dependencies...")

# --- 1. Boost: Use system-installed version (Boost is too heavy to build from source) ---
# User must install: sudo apt-get install libboost-all-dev (Linux)
find_package(Boost REQUIRED COMPONENTS system date_time)

# --- 2. fmt: Automatically fetch and build (lightweight) ---
message(STATUS "Fetching fmt...")
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.2.1
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE # Magic: other libraries calling find_package(fmt) will use this instance
)
FetchContent_MakeAvailable(fmt)

# --- 3. spdlog: Automatically fetch and build (lightweight) ---
message(STATUS "Fetching spdlog...")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.13.0
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE
)
# Force spdlog to use the previously fetched fmt instead of building its own
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(spdlog)

# --- 4. yaml-cpp: Automatically fetch and build (lightweight) ---
message(STATUS "Fetching yaml-cpp...")
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        0.8.0
    GIT_SHALLOW    TRUE
    OVERRIDE_FIND_PACKAGE
)
# Disable yaml-cpp tests and tools to keep build lightweight
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

# --- 5. Catch2: Testing framework ---
message(STATUS "Fetching Catch2...")
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

#####                   SystemC                       #####
message(STATUS "Fetching SystemC from GitHub...")
FetchContent_Declare(
    SystemC
    GIT_REPOSITORY https://github.com/accellera-official/systemc.git
    GIT_TAG        2.3.4
    GIT_SHALLOW    TRUE
)
# SystemC build options
set(ENABLE_PHASE_CALLBACKS OFF CACHE BOOL "" FORCE)
set(ENABLE_ASSERTIONS ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SystemC)

#####                   SCC (SystemC-Components)      #####
message(STATUS "Fetching SCC (SystemC-Components)...")
FetchContent_Declare(
    scc
    GIT_REPOSITORY https://github.com/Minres/SystemC-Components.git
    GIT_TAG        2024.07
    GIT_SHALLOW    TRUE
    # Patch to prevent SCC from searching SystemC via find_package
    PATCH_COMMAND  sh -c "echo 'if(TARGET SystemC::systemc)\n  return()\nendif()' | cat - cmake/SystemCPackage.cmake > cmake/SystemCPackage.cmake.tmp && mv cmake/SystemCPackage.cmake.tmp cmake/SystemCPackage.cmake"
)
# SCC build options
set(BUILD_SCC_LIB_ONLY ON CACHE BOOL "" FORCE)
set(SC_WITH_PHASE_CALLBACKS OFF CACHE BOOL "" FORCE)
set(SC_WITH_PHASE_CALLBACK_TRACING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(scc)

#####                   Compiler Options              #####
if(ENABLE_SANITIZER)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

#####                   Subdirectories                #####
add_subdirectory(src)
add_subdirectory(testbench)
