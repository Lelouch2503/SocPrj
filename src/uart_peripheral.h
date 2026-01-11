#ifndef UART_PERIPHERAL_H
#define UART_PERIPHERAL_H

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <scc/register.h>
#include <scc/utilities.h>

#include <cstdint>
#include <functional>
#include <array>

/**
 * @brief UART Peripheral with register callbacks
 * 
 * Register Map:
 * - 0x00: UART_CTRL  (Control Register)
 * - 0x04: UART_STAT  (Status Register) 
 * - 0x08: UART_TX    (Transmit Data Register)
 * - 0x0C: UART_RX    (Receive Data Register)
 */
class UartPeripheral : public sc_core::sc_module {
public:
    // TLM2.0 target socket for bus access
    tlm_utils::simple_target_socket<UartPeripheral> socket;

    // Register indices
    enum RegIndex : uint32_t {
        UART_CTRL = 0,
        UART_STAT = 1,
        UART_TX   = 2,
        UART_RX   = 3,
        REG_COUNT = 4
    };

    // Register addresses
    static constexpr uint64_t ADDR_CTRL = 0x00;
    static constexpr uint64_t ADDR_STAT = 0x04;
    static constexpr uint64_t ADDR_TX   = 0x08;
    static constexpr uint64_t ADDR_RX   = 0x0C;

    // Control register bits
    static constexpr uint32_t CTRL_ENABLE     = (1 << 0);
    static constexpr uint32_t CTRL_TX_IE      = (1 << 1);  // TX interrupt enable
    static constexpr uint32_t CTRL_RX_IE      = (1 << 2);  // RX interrupt enable

    // Status register bits
    static constexpr uint32_t STAT_TX_READY   = (1 << 0);
    static constexpr uint32_t STAT_RX_VALID   = (1 << 1);
    static constexpr uint32_t STAT_TX_BUSY    = (1 << 2);

    SC_HAS_PROCESS(UartPeripheral);
    
    explicit UartPeripheral(sc_core::sc_module_name name);
    ~UartPeripheral() override = default;

    // Callback type for user-defined handlers
    using WriteCallback = std::function<void(uint32_t reg_index, uint32_t value)>;
    using ReadCallback = std::function<void(uint32_t reg_index, uint32_t& value)>;

    // Register user callbacks
    void set_write_callback(WriteCallback cb) { m_write_callback = std::move(cb); }
    void set_read_callback(ReadCallback cb) { m_read_callback = std::move(cb); }

    // Direct register access (for internal use or testing)
    uint32_t get_register(RegIndex idx) const { return m_registers[idx]; }
    void set_register(RegIndex idx, uint32_t value) { m_registers[idx] = value; }

private:
    // TLM2.0 blocking transport
    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

    // Internal register read/write handlers
    void handle_write(uint64_t addr, uint32_t value);
    uint32_t handle_read(uint64_t addr);

    // Register-specific callbacks (called when accessed via TLM)
    void on_ctrl_write(uint32_t value);
    void on_tx_write(uint32_t value);
    uint32_t on_rx_read();

    // Register storage
    std::array<uint32_t, REG_COUNT> m_registers;

    // User callbacks
    WriteCallback m_write_callback;
    ReadCallback m_read_callback;
};

#endif // UART_PERIPHERAL_H
