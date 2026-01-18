#include "uart_reg.h"
#include <spdlog/spdlog.h>

UartReg::UartReg(std::string name)
    : m_registers{}, m_name(std::move(name))
{
    // Initialize registers with default values
    m_registers[UART_CTRL] = 0x00000000;
    m_registers[UART_STAT] = STAT_TX_READY;  // TX ready by default
    m_registers[UART_TX]   = 0x00000000;
    m_registers[UART_RX]   = 0x00000000;
}

void UartReg::handle_write(uint64_t addr, uint32_t value)
{
    uint32_t reg_index = static_cast<uint32_t>(addr / 4);
    
    spdlog::info("[{}] WRITE to reg[{}] @ 0x{:02X} = 0x{:08X}", 
                 m_name, reg_index, addr, value);

    switch (addr) {
        case ADDR_CTRL:
            on_ctrl_write(value);
            break;
        case ADDR_STAT:
            on_stat_write(value);
            break;
        case ADDR_TX:
            on_tx_write(value);
            break;
        case ADDR_RX:
            // RX register is read-only, ignore writes
            spdlog::warn("[{}] Attempted write to RX register (read-only)", m_name);
            break;
        default:
            spdlog::warn("[{}] Write to unknown address 0x{:02X}", m_name, addr);
            break;
    }
}

uint32_t UartReg::handle_read(uint64_t addr)
{
    uint32_t reg_index = static_cast<uint32_t>(addr / 4);
    uint32_t value = 0;

    switch (addr) {
        case ADDR_CTRL:
            value = m_registers[UART_CTRL];
            break;
        case ADDR_STAT:
            value = m_registers[UART_STAT];
            break;
        case ADDR_TX:
            value = m_registers[UART_TX];
            break;
        case ADDR_RX:
            value = on_rx_read();
            break;
        default:
            value = 0;
            spdlog::warn("[{}] Read from unknown address 0x{:02X}", m_name, addr);
            break;
    }

    spdlog::info("[{}] READ from reg[{}] @ 0x{:02X} = 0x{:08X}", 
                 m_name, reg_index, addr, value);

    return value;
}

void UartReg::on_stat_write(uint32_t value)
{
    // Status register is typically read-only, but allow write for testing
    m_registers[UART_STAT] = value;
    spdlog::warn("[{}] Writing to status register (typically read-only)", m_name);
}
