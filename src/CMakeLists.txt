add_library(soc_peripherals STATIC
    uart_peripheral.cpp
    uart_reg.cpp
)

target_include_directories(soc_peripherals PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/_deps/scc-src/src
    ${CMAKE_BINARY_DIR}/_deps/systemc-src/src
)

target_link_libraries(soc_peripherals PUBLIC
    SystemC::systemc
    scc::scc-sysc
    fmt::fmt
    spdlog::spdlog
)
