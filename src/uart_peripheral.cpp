#include "uart_peripheral.h"
#include <spdlog/spdlog.h>
#include <fmt/format.h>

UartPeripheral::UartPeripheral(sc_core::sc_module_name name)
    : sc_module(name)
    , socket("socket")
    , m_registers{}
{
    // Register TLM2.0 blocking transport callback
    socket.register_b_transport(this, &UartPeripheral::b_transport);

    // Initialize registers with default values
    m_registers[UART_CTRL] = 0x00000000;
    m_registers[UART_STAT] = STAT_TX_READY;  // TX ready by default
    m_registers[UART_TX]   = 0x00000000;
    m_registers[UART_RX]   = 0x00000000;

    spdlog::info("[{}] UART Peripheral initialized", name);
}

void UartPeripheral::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
{
    tlm::tlm_command cmd = trans.get_command();
    uint64_t addr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();

    // Validate address alignment and length
    if (addr % 4 != 0 || len != 4) {
        spdlog::error("[{}] Unaligned access or invalid length at addr 0x{:08X}", 
                      name(), addr);
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    // Validate address range
    if (addr >= (REG_COUNT * 4)) {
        spdlog::error("[{}] Address out of range: 0x{:08X}", name(), addr);
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    if (cmd == tlm::TLM_WRITE_COMMAND) {
        uint32_t value = *reinterpret_cast<uint32_t*>(ptr);
        handle_write(addr, value);
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }
    else if (cmd == tlm::TLM_READ_COMMAND) {
        uint32_t value = handle_read(addr);
        *reinterpret_cast<uint32_t*>(ptr) = value;
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }
    else {
        trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    }
}

void UartPeripheral::handle_write(uint64_t addr, uint32_t value)
{
    uint32_t reg_index = static_cast<uint32_t>(addr / 4);
    
    spdlog::info("[{}] WRITE to reg[{}] @ 0x{:02X} = 0x{:08X}", 
                 name(), reg_index, addr, value);

    switch (addr) {
        case ADDR_CTRL:
            on_ctrl_write(value);
            break;
        case ADDR_STAT:
            // Status register is typically read-only, but allow write for testing
            m_registers[UART_STAT] = value;
            spdlog::warn("[{}] Writing to status register (typically read-only)", name());
            break;
        case ADDR_TX:
            on_tx_write(value);
            break;
        case ADDR_RX:
            // RX register is read-only, ignore writes
            spdlog::warn("[{}] Attempted write to RX register (read-only)", name());
            break;
        default:
            break;
    }

    // Call user callback if registered
    if (m_write_callback) {
        m_write_callback(reg_index, value);
    }
}

uint32_t UartPeripheral::handle_read(uint64_t addr)
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
            break;
    }

    spdlog::info("[{}] READ from reg[{}] @ 0x{:02X} = 0x{:08X}", 
                 name(), reg_index, addr, value);

    // Call user callback if registered
    if (m_read_callback) {
        m_read_callback(reg_index, value);
    }

    return value;
}

void UartPeripheral::on_ctrl_write(uint32_t value)
{
    m_registers[UART_CTRL] = value;

    if (value & CTRL_ENABLE) {
        spdlog::info("[{}] UART ENABLED", name());
    } else {
        spdlog::info("[{}] UART DISABLED", name());
    }

    if (value & CTRL_TX_IE) {
        spdlog::debug("[{}] TX interrupt enabled", name());
    }
    if (value & CTRL_RX_IE) {
        spdlog::debug("[{}] RX interrupt enabled", name());
    }
}

void UartPeripheral::on_tx_write(uint32_t value)
{
    m_registers[UART_TX] = value;
    
    // Simulate transmission
    char tx_char = static_cast<char>(value & 0xFF);
    spdlog::info("[{}] TX: Sending character '{}' (0x{:02X})", 
                 name(), (tx_char >= 32 && tx_char < 127) ? tx_char : '?', value & 0xFF);

    // Set TX busy, clear TX ready
    m_registers[UART_STAT] &= ~STAT_TX_READY;
    m_registers[UART_STAT] |= STAT_TX_BUSY;

    // In a real implementation, you would:
    // 1. Start a process to simulate transmission delay
    // 2. Set TX ready after transmission completes
    // For simplicity, we immediately set TX ready again
    m_registers[UART_STAT] |= STAT_TX_READY;
    m_registers[UART_STAT] &= ~STAT_TX_BUSY;
}

uint32_t UartPeripheral::on_rx_read()
{
    uint32_t value = m_registers[UART_RX];
    
    // Clear RX valid after read
    m_registers[UART_STAT] &= ~STAT_RX_VALID;
    
    spdlog::info("[{}] RX: Read character (0x{:02X})", name(), value & 0xFF);
    
    return value;
}
