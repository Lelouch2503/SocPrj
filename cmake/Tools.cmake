function(add_cmake_format_target)
    if(NOT $(ENABLE_CMAKE_FORMAT))
        message(WARNING "Cmake format is not enable")
        return()
    endif()

    set(ROOT_CMAKE_FILES "${CMAKE_SOURCE_DIR}/CMakeLists.txt")
    file(GLOB_RECURSE CMAKE_LIST_TXT "*/CMakeLists.txt")
    file(GLOB_RECURSE CMAKE_FILES_C  "cmake/*.cmake")
    list(
        FILTER
        CMAKE_FILES_TXT
        EXCLUDE
        REGEX
        "${CMAKE_SOURCE_DIR}/(build|external)/.*"
    )
    set(CMAKE_FILES ${ROOT_CMAKE_FILES} ${CMAKE_FILES_TXT} ${CMAKE_FILES_C})
    find_program(CMAKE_FORMAT cmake-format)

    if(CMAKE_FORMAT)
        message(STATUS "Added Cmake Format")
        set(FORMATING_COMMANDS)
        foreach(cmake_file ${CMAKE_FILES})
            list(
                APPEND
                FORMATTING_COMMANDS
                COMMAND
                cmake-format
                -c
                ${CMAKE_SOURCE_DIR}
                -i
                ${cmake_file}
            )
        endforeach()
        add_custom_target(
            run_cmake_format
            COMMAND ${FORMATTING_COMMANDS}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
    else()
        message(WARNING "CMAKE_FORMAT NOT FOUND")
    endif()
endfunction()

function(add_clang_format_target)
    if(NOT $(ENABLE_CLANG_FORMAT))
        message(WARNING "Clang format is not enable")
        return()
    endif()

    find_package(Python3 COMPONENTS Interpreter)

    if(NOT ${Python_FOUND})
        return()
    endif()

    file(GLOB_RECURSE CMAKE_FILES_CC "*/*.cc")
    file(GLOB_RECURSE CMAKE_FILES_CPP "*/*.cpp")
    file(GLOB_RECURSE CMAKE_FILES_H "*/*.h")
    file(GLOB_RECURSE CMAKE_FILES_HPP "*/*.hpp")

    set(CPP_FILES
        ${CMAKE_FILES_CC}
        ${CMAKE_FILES_CPP}
        ${CMAKE_FILES_H}
        ${CMAKE_FILES_HPP})
    list(
        FILTER
        CPP_FILES
        EXCLUDE
        REGEX
        "${CMAKE_SOURCE_DIR}/(build|external)/.*")

    find_program(CMAKE_FORMAT cmake-format)

    find_program(CLANGFORMAT clang-format)
    if(CLANGFORMAT)
        message(STATUS "Added Clang Format")
        add_custom_target(
            run_clang_format
            COMMAND
                ${Python3_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/tools/run-clang-format.py ${CPP_FILES}
                --in-place
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            USES_TERMINAL)
    else()
        message(WARNING "CLANGFORMAT NOT FOUND")
    endif()
endfunction()
