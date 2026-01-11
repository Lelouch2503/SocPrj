#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "uart_peripheral.h"

/**
 * @brief Simple TLM2.0 Initiator for testing the UART peripheral
 */
class TestInitiator : public sc_core::sc_module {
public:
    tlm_utils::simple_initiator_socket<TestInitiator> socket;

    SC_HAS_PROCESS(TestInitiator);

    explicit TestInitiator(sc_core::sc_module_name name)
        : sc_module(name)
        , socket("socket")
    {
        SC_THREAD(run_tests);
    }

    void run_tests() {
        spdlog::info("========================================");
        spdlog::info("       UART Register Test Begin        ");
        spdlog::info("========================================");

        // Test 1: Write to CTRL register (Enable UART)
        spdlog::info("\n--- Test 1: Enable UART ---");
        write_reg(UartPeripheral::ADDR_CTRL, 
                  UartPeripheral::CTRL_ENABLE | UartPeripheral::CTRL_TX_IE);

        // Test 2: Read STAT register
        spdlog::info("\n--- Test 2: Read Status Register ---");
        uint32_t status = read_reg(UartPeripheral::ADDR_STAT);
        if (status & UartPeripheral::STAT_TX_READY) {
            spdlog::info("TX is ready!");
        }

        // Test 3: Write to TX register (Send 'A')
        spdlog::info("\n--- Test 3: Send Character 'A' ---");
        write_reg(UartPeripheral::ADDR_TX, 'A');

        // Test 4: Write to TX register (Send 'B')
        spdlog::info("\n--- Test 4: Send Character 'B' ---");
        write_reg(UartPeripheral::ADDR_TX, 'B');

        // Test 5: Write to TX register (Send 'C')
        spdlog::info("\n--- Test 5: Send Character 'C' ---");
        write_reg(UartPeripheral::ADDR_TX, 'C');

        // Test 6: Read RX register
        spdlog::info("\n--- Test 6: Read RX Register ---");
        uint32_t rx_data = read_reg(UartPeripheral::ADDR_RX);
        spdlog::info("Received data: 0x{:02X}", rx_data);

        // Test 7: Disable UART
        spdlog::info("\n--- Test 7: Disable UART ---");
        write_reg(UartPeripheral::ADDR_CTRL, 0x00);

        spdlog::info("\n========================================");
        spdlog::info("       UART Register Test Complete     ");
        spdlog::info("========================================");

        sc_core::sc_stop();
    }

private:
    void write_reg(uint64_t addr, uint32_t value) {
        tlm::tlm_generic_payload trans;
        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

        trans.set_command(tlm::TLM_WRITE_COMMAND);
        trans.set_address(addr);
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&value));
        trans.set_data_length(sizeof(value));
        trans.set_streaming_width(sizeof(value));
        trans.set_byte_enable_ptr(nullptr);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        socket->b_transport(trans, delay);

        if (trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
            spdlog::error("Write failed at address 0x{:02X}", addr);
        }
    }

    uint32_t read_reg(uint64_t addr) {
        tlm::tlm_generic_payload trans;
        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
        uint32_t value = 0;

        trans.set_command(tlm::TLM_READ_COMMAND);
        trans.set_address(addr);
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&value));
        trans.set_data_length(sizeof(value));
        trans.set_streaming_width(sizeof(value));
        trans.set_byte_enable_ptr(nullptr);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        socket->b_transport(trans, delay);

        if (trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
            spdlog::error("Read failed at address 0x{:02X}", addr);
        }

        return value;
    }
};

int sc_main(int argc, char* argv[]) {
    // Setup spdlog
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

    spdlog::info("SystemC UART Testbench Starting...");
    spdlog::info("SystemC Version: {}", sc_core::sc_release());

    // Instantiate modules
    UartPeripheral uart("uart0");
    TestInitiator  initiator("initiator");

    // Register custom callback to demonstrate user-defined behavior
    uart.set_write_callback([](uint32_t reg_index, uint32_t value) {
        spdlog::debug("[USER CALLBACK] Write to reg[{}] = 0x{:08X}", reg_index, value);
    });

    uart.set_read_callback([](uint32_t reg_index, uint32_t& value) {
        spdlog::debug("[USER CALLBACK] Read from reg[{}] = 0x{:08X}", reg_index, value);
    });

    // Connect initiator to UART target
    initiator.socket.bind(uart.socket);

    // Run simulation
    spdlog::info("Starting simulation...\n");
    sc_core::sc_start();

    spdlog::info("\nSimulation finished at time: {}", 
                 sc_core::sc_time_stamp().to_string());

    return 0;
}
