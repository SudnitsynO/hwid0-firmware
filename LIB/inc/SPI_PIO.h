#pragma once
#include "SPI_base.h"
#include "delay.h"
#include "gpio_interface.h"
#include <cstdint>


enum class ClockPhase
{
    BEGIN,
    MIDDLE
};

enum class ClockPolarity
{
    LOW_IDLE,
    HIGH_IDLE
};

enum class BitOrder
{
    LSB_FIRST,
    MSB_FIRST
};

template <class SCLK, class MISO, class MOSI, int DELAY, ClockPolarity POLARITY, ClockPhase PHASE, BitOrder BIT_ORDER>
class SpiPio : public SPI_MasterBase
{
  public:
    uint8_t test_data[10] = {};
    void test();
    uint8_t tx_byte(const uint8_t byte) override;
    void tx_rx_buf(const uint8_t *const tx_buffer, uint8_t *const rx_buffer, const size_t data_count) override;
    void tx_buf(const uint8_t *const tx_buffer, const size_t data_count) override;
    void rx_buf(uint8_t *const rx_buffer, const size_t data_count) override;
    void flush() override;
    void init();
};

template <class SCLK, class MISO, class MOSI, int DELAY, ClockPolarity POLARITY, ClockPhase PHASE, BitOrder BIT_ORDER>
void SpiPio<SCLK, MISO, MOSI, DELAY, POLARITY, PHASE, BIT_ORDER>::init()
{
    SCLK::SetMode(Mode::Output, false);
    MISO::SetMode(Mode::Input);
    MOSI::SetMode(Mode::Output, false);
}

template <class SCLK, class MISO, class MOSI, int DELAY, ClockPolarity POLARITY, ClockPhase PHASE, BitOrder BIT_ORDER>
uint8_t SpiPio<SCLK, MISO, MOSI, DELAY, POLARITY, PHASE, BIT_ORDER>::tx_byte(const uint8_t byte)
{
    uint8_t tx_data = byte;
    uint8_t rx_data = 0;

    if (POLARITY == ClockPolarity::HIGH_IDLE)
        SCLK::Set();
    else
        SCLK::Reset();

    if (PHASE == ClockPhase::BEGIN)
    {
        if (BIT_ORDER == BitOrder::LSB_FIRST)
        {
            for (size_t i = 0; i < 8; i++)
            {
                MOSI::Write(tx_data & 1);
                if (DELAY != 0)
                    Delay_us(DELAY);
                SCLK::Toggle();
                rx_data = (rx_data >> 1) | (MISO::Read() ? 0x80 : 0);
                if (DELAY != 0)
                    Delay_us(DELAY);
                SCLK::Toggle();
                tx_data >>= 1;
            }
        }
        else if (BIT_ORDER == BitOrder::MSB_FIRST)
        {
            for (size_t i = 0; i < 8; i++)
            {
                MOSI::Write(tx_data & 0x80);
                if (DELAY != 0)
                    Delay_us(DELAY);
                SCLK::Toggle();
                rx_data = (rx_data << 1) | (MISO::Read() ? 0x1 : 0);
                if (DELAY != 0)
                    Delay_us(DELAY);
                SCLK::Toggle();
                tx_data <<= 1;
            }
        }
    }
    else if (PHASE == ClockPhase::MIDDLE)
    {
        if (BIT_ORDER == BitOrder::LSB_FIRST)
        {
            for (size_t i = 0; i < 8; i++)
            {
                SCLK::Toggle();
                MOSI::Write(tx_data & 1);
                if (DELAY != 0)
                    Delay_us(DELAY);
                SCLK::Toggle();
                rx_data = (rx_data >> 1) | (MISO::Read() ? 0x80 : 0);
                if (DELAY != 0)
                    Delay_us(DELAY);
                tx_data >>= 1;
            }
        }
        else if (BIT_ORDER == BitOrder::MSB_FIRST)
        {
            for (size_t i = 0; i < 8; i++)
            {
                SCLK::Toggle();
                MOSI::Write(tx_data & 0x80);
                if (DELAY != 0)
                    Delay_us(DELAY);
                SCLK::Toggle();
                rx_data = (rx_data << 1) | (MISO::Read() ? 1 : 0);
                if (DELAY != 0)
                    Delay_us(DELAY);
                tx_data <<= 1;
            }
        }
    }
    return rx_data;
}

template <class SCLK, class MISO, class MOSI, int DELAY, ClockPolarity POLARITY, ClockPhase PHASE, BitOrder BIT_ORDER>
void SpiPio<SCLK, MISO, MOSI, DELAY, POLARITY, PHASE, BIT_ORDER>::tx_rx_buf(const uint8_t *const tx_buffer,
                                                                            uint8_t *const rx_buffer,
                                                                            const size_t data_count)
{
    for (size_t i = 0; i < data_count; i++)
    {
        rx_buffer[i] = tx_byte(tx_buffer[i]);
    }
}

template <class SCLK, class MISO, class MOSI, int DELAY, ClockPolarity POLARITY, ClockPhase PHASE, BitOrder BIT_ORDER>
void SpiPio<SCLK, MISO, MOSI, DELAY, POLARITY, PHASE, BIT_ORDER>::tx_buf(const uint8_t *const tx_buffer,
                                                                         const size_t data_count)
{
    for (size_t i = 0; i < data_count; i++)
    {
        tx_byte(tx_buffer[i]);
    }
}

template <class SCLK, class MISO, class MOSI, int DELAY, ClockPolarity POLARITY, ClockPhase PHASE, BitOrder BIT_ORDER>
void SpiPio<SCLK, MISO, MOSI, DELAY, POLARITY, PHASE, BIT_ORDER>::rx_buf(uint8_t *const rx_buffer,
                                                                         const size_t data_count)
{
    for (size_t i = 0; i < data_count; i++)
    {
        rx_buffer[i] = tx_byte(0);
    }
}

template <class SCLK, class MISO, class MOSI, int DELAY, ClockPolarity POLARITY, ClockPhase PHASE, BitOrder BIT_ORDER>
void SpiPio<SCLK, MISO, MOSI, DELAY, POLARITY, PHASE, BIT_ORDER>::flush()
{
}
