cmake_minimum_required(VERSION 3.20...3.30)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0")
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
endif()

#####                   Options                      #####
option(USE_CONAN "Whether to use Conan." OFF)
option(USE_VCPKG "Whether to use VCPKG." OFF)
option(USE_CPM "Whether to use CPM." ON)
project(SystemC_SoC VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BUILD_SHARED_LIBS OFF)

option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(ENABLE_SANITIZER "Enable Address and Undefined Sanitizers" OFF)
option(ENABLE_SANITIZE_ADDR "Enable address sanitizer." OFF)
option(ENABLE_SANITIZE_UNDEF "Enable undefined behavior sanitizer." OFF)
option(ENABLE_SANITIZE_LEAK "Enable leak sanitizer (GCC/Clang only)." OFF)
option(ENABLE_SANITIZE_THREAD "Enable thread sanitizer (GCC/Clang only)." OFF)
option(ENABLE_CMAKE_FORMAT "Enable cmake-format integration." OFF)
option(ENABLE_TESTING "Enable test" OFF)

#####                   CMake Modules                #####
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(AddGitSubmodule)
include(FetchContent)
include(Docs)
set(FETCHCONTENT_QUIET OFF)

#####                   Dependencies                  #####

if(USE_CPM)
    message(STATUS "Using CPM for dependency management")
    include(cmake/CPM.cmake)

    # JSON
    CPMAddPackage("gh:nlohmann/json#v3.11.3")

    # fmt
    CPMAddPackage("gh:fmtlib/fmt#9.1.0")

    # spdlog
    CPMAddPackage(
        NAME spdlog
        GITHUB_REPOSITORY gabime/spdlog
        GIT_TAG v1.13.0
        OPTIONS "SPDLOG_FMT_EXTERNAL ON"
    )

    # yaml-cpp
    CPMAddPackage(
        NAME yaml-cpp
        GITHUB_REPOSITORY jbeder/yaml-cpp
        GIT_TAG 0.8.0
        OPTIONS "YAML_CPP_BUILD_TESTS OFF" "YAML_CPP_BUILD_TOOLS OFF"
    )

    # cxxopts
    CPMAddPackage("gh:jarro2783/cxxopts#v3.1.1")

    # Boost
    find_package(Boost REQUIRED COMPONENTS system date_time)

    # SystemC
    CPMAddPackage(
        NAME SystemC
        GITHUB_REPOSITORY accellera-official/systemc
        GIT_TAG 2.3.4
        OPTIONS "ENABLE_PHASE_CALLBACKS OFF" "ENABLE_ASSERTIONS ON" "BUILD_TESTING OFF"
    )

    # SCC (SystemC Components)
    # Tell SCC that SystemC is already available to prevent find_package() call
    set(SystemC_FOUND TRUE CACHE BOOL "" FORCE)
    set(SystemC_DIR "${SystemC_BINARY_DIR}" CACHE PATH "" FORCE)
    set(SYSTEMC_TARGET_ARCH "linux64" CACHE STRING "" FORCE)
    
    # Disable SCC installation to prevent export target errors
    set(SCC_INSTALL_COMPONENTS OFF CACHE BOOL "" FORCE)
    
    CPMAddPackage(
        NAME scc
        GITHUB_REPOSITORY Minres/SystemC-Components
        GIT_TAG 2024.07
        OPTIONS 
            "BUILD_SCC_LIB_ONLY ON" 
            "SC_WITH_PHASE_CALLBACKS OFF"
            "CMAKE_SKIP_INSTALL_RULES ON" 
        EXCLUDE_FROM_ALL YES
    )

    # Catch2
    if(ENABLE_TESTING)
        CPMAddPackage("gh:catchorg/Catch2#v3.5.2")
    endif()

elseif(USE_CONAN OR USE_VCPKG)
    if(USE_CONAN)
        message(STATUS "Using Conan for dependency management")
        include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    elseif(USE_VCPKG)
        message(STATUS "Using VCPKG for dependency management")
    endif()

    find_package(nlohmann_json REQUIRED)
    find_package(fmt REQUIRED)
    find_package(spdlog REQUIRED)
    find_package(cxxopts REQUIRED)
    find_package(yaml-cpp REQUIRED)
    find_package(Boost REQUIRED COMPONENTS system date_time)
    find_package(SystemC REQUIRED)

    # SCC
    find_package(SCC QUIET)
    if(NOT SCC_FOUND)
        message(WARNING "SCC not found via package manager. Fallback to FetchContent/CPM recommended here.")
        message(STATUS "Fetching SCC (SystemC-Components)...")
        FetchContent_Declare(
            scc
            GIT_REPOSITORY https://github.com/Minres/SystemC-Components.git
            GIT_TAG        2024.07
            GIT_SHALLOW    TRUE
            PATCH_COMMAND  sh -c "echo 'if(TARGET SystemC::systemc)\n  return()\nendif()' | cat - cmake/SystemCPackage.cmake > cmake/SystemCPackage.cmake.tmp && mv cmake/SystemCPackage.cmake.tmp cmake/SystemCPackage.cmake"
        )
        set(BUILD_SCC_LIB_ONLY ON CACHE BOOL "" FORCE)
        set(SC_WITH_PHASE_CALLBACKS OFF CACHE BOOL "" FORCE)
        set(SC_WITH_PHASE_CALLBACK_TRACING OFF CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(scc)
    endif()

    if(ENABLE_TESTING)
        find_package(Catch2 REQUIRED)
        if(Catch2_SOURCE_DIR)
            list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
        endif()
    endif()

else()
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        set(FC_OVERRIDE_FIND_PACKAGE OVERRIDE_FIND_PACKAGE)
    else()
        set(FC_OVERRIDE_FIND_PACKAGE "")
    endif()
    FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts
        GIT_TAG v3.1.1
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(cxxopts)

    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(nlohmann_json)

    find_package(Boost REQUIRED COMPONENTS system date_time)

    # fmt
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        9.1.0
        GIT_SHALLOW    TRUE
        ${FC_OVERRIDE_FIND_PACKAGE}
    )
    FetchContent_MakeAvailable(fmt)

    # spdlog
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.13.0
        GIT_SHALLOW    TRUE
        ${FC_OVERRIDE_FIND_PACKAGE}
    )
    set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(spdlog)

    # yaml-cpp
    FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG        0.8.0
        GIT_SHALLOW    TRUE
        ${FC_OVERRIDE_FIND_PACKAGE}
    )
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(yaml-cpp)

    # Catch2
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    # SystemC
    FetchContent_Declare(
        SystemC
        GIT_REPOSITORY https://github.com/accellera-official/systemc.git
        GIT_TAG        2.3.4
        GIT_SHALLOW    TRUE
    )
    set(ENABLE_PHASE_CALLBACKS OFF CACHE BOOL "" FORCE)
    set(ENABLE_ASSERTIONS ON CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SystemC)

    # SCC (SystemC-Components)
    FetchContent_Declare(
        scc
        GIT_REPOSITORY https://github.com/Minres/SystemC-Components.git
        GIT_TAG        2024.07
        GIT_SHALLOW    TRUE
        PATCH_COMMAND  sh -c "echo 'if(TARGET SystemC::systemc)\n  return()\nendif()' | cat - cmake/SystemCPackage.cmake > cmake/SystemCPackage.cmake.tmp && mv cmake/SystemCPackage.cmake.tmp cmake/SystemCPackage.cmake"
    )
    set(BUILD_SCC_LIB_ONLY ON CACHE BOOL "" FORCE)
    set(SC_WITH_PHASE_CALLBACKS OFF CACHE BOOL "" FORCE)
    set(SC_WITH_PHASE_CALLBACK_TRACING OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(scc)

endif()


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
