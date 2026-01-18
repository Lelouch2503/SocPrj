#ifndef UART_PERIPHERAL_H
#define UART_PERIPHERAL_H

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include <cstdint>
#include <functional>
#include <array>

#include "uart_reg.h"

/**
 * @brief UART Peripheral with register callbacks
 */
class UartPeripheral : public sc_core::sc_module, public UartReg {
public:
    // TLM2.0 target socket for bus access
    tlm_utils::simple_target_socket<UartPeripheral> socket;

    SC_HAS_PROCESS(UartPeripheral);
    
    explicit UartPeripheral(sc_core::sc_module_name name);
    ~UartPeripheral() override = default;

    // Callback type for user-defined handlers
    using WriteCallback = std::function<void(uint32_t reg_index, uint32_t value)>;
    using ReadCallback = std::function<void(uint32_t reg_index, uint32_t& value)>;

    // Register user callbacks
    void set_write_callback(WriteCallback cb) { m_write_callback = std::move(cb); }
    void set_read_callback(ReadCallback cb) { m_read_callback = std::move(cb); }

private:
    // TLM2.0 blocking transport
    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

    // Register-specific callbacks (overriding UartReg pure virtuals)
    void on_ctrl_write(uint32_t value) override;
    void on_tx_write(uint32_t value) override;
    uint32_t on_rx_read() override;

    // User callbacks
    WriteCallback m_write_callback;
    ReadCallback m_read_callback;
};

#endif // UART_PERIPHERAL_H
