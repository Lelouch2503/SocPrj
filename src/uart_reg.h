#ifndef UART_REG_H
#define UART_REG_H

#include <cstdint>
#include <array>
#include <string>
#include <functional>

/**
 * @brief UART Register Interface
 * 
 * Register Map:
 * - 0x00: UART_CTRL  (Control Register)
 * - 0x04: UART_STAT  (Status Register) 
 * - 0x08: UART_TX    (Transmit Data Register)
 * - 0x0C: UART_RX    (Receive Data Register)
 */
class UartReg {
public:
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

    // Constructor with name for logging purposes
    explicit UartReg(std::string name);
    virtual ~UartReg() = default;

    // Register access handlers
    void handle_write(uint64_t addr, uint32_t value);
    uint32_t handle_read(uint64_t addr);

    // Direct access
    uint32_t get_register(RegIndex idx) const { return m_registers[idx]; }
    void set_register(RegIndex idx, uint32_t value) { m_registers[idx] = value; }

protected:
    // Virtual callbacks for register access
    virtual void on_ctrl_write(uint32_t value) = 0;
    virtual void on_tx_write(uint32_t value) = 0;
    virtual uint32_t on_rx_read() = 0;
    
    // Optional: Virtual callback for status write (useful for testing)
    // Default implementation can just write to the register
    virtual void on_stat_write(uint32_t value);

    // Register storage
    std::array<uint32_t, REG_COUNT> m_registers;
    
    // Name for logging
    std::string m_name;
};

#endif // UART_REG_H
