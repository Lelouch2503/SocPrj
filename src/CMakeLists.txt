add_library(soc_peripherals STATIC
    uart_peripheral.cpp
)

target_include_directories(soc_peripherals PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(soc_peripherals PUBLIC
    SystemC::systemc
    scc::scc
    fmt::fmt
    spdlog::spdlog
)
