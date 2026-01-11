add_executable(tb_uart
    tb_uart.cpp
)

target_link_libraries(tb_uart PRIVATE
    soc_peripherals
    SystemC::systemc
    spdlog::spdlog
)

# Add test
add_test(NAME uart_test COMMAND tb_uart)
