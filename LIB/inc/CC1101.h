/************************************************************
 *					CC1101 library
 *	Author: UN7PCS
 *	Date: 2022
 *  1) Настроить конфигурацию трансивера по шаблону xml
 *  2) передать конфиг в rf_init
 *
 ************************************************************/
#pragma once
#include "Buffer.h"
#include "delay.h"
#include "gpio_interface.h"
#include "software_timer_interface.h"
#include <array>
#include <cstddef>
#include <memory>
#include <stdint.h>

//#include "CubePinF0.h"

template <class MOSI, class MISO, class SCLK, class CS> class CC1101
{

  public:
    typedef std::pair<uint16_t, uint8_t> RegValue;
    Delegate<void()> rx_error;
    Delegate<void()> tx_error;
    Delegate<void()> tx_complete;
    Delegate<void(Buffer &data, int rssi, int liq)> rx_complete;

  public:
    template <typename RIL> void init(RIL reg_init_list)
    {
        MOSI::Init();
        MISO::Init();
        SCLK::Init();
        CS::Init();
        MOSI::SetMode(Mode::Output);
        MISO::SetMode(Mode::Input);
        SCLK::SetMode(Mode::Output);
        CS::SetMode(Mode::Output);

        MISO::DisableInterrupt();

        timeout_timer = SoftwareTimer::create_timer();

        for (RegValue val : reg_init_list)
        {
            write_reg(val.first, val.second);
        }
    }

    void start_rx(int timeout = 0)
    {
        MISO::DisableInterrupt();
        timeout_timer->stop();
        if (timeout)
        {
            timeout_timer->tick.init<CC1101<MOSI, MISO, SCLK, CS>, &CC1101<MOSI, MISO, SCLK, CS>::rx_timeout_isr>(this);
            timeout_timer->start_one_shoot(timeout);
        }
        wait_for_idle();                                // stop RF
        send_cmd(Commands::SFRX);                       // flush RF buffer
        send_cmd(Commands::SRX);                        // set rx mode
        MISO::SetInterrupt(InterruptMode::FallingEdge); // enable MISO IRQ
        MISO::GetIsrDelegate()
            .template init<CC1101<MOSI, MISO, SCLK, CS>, &CC1101<MOSI, MISO, SCLK, CS>::rx_complete_isr>(
                this); // activate ISR
    }

    void stop()
    {
        MISO::DisableInterrupt();
        wait_for_idle();
        timeout_timer->stop();
    }

    void start_tx(const uint8_t *buffer, const uint8_t data_len)
    {
        MISO::DisableInterrupt();
        wait_for_idle();
        send_cmd(Commands::SFRX);
        send_cmd(Commands::SFTX);
        if (data_len > 50)
            return;
        write_reg(0x3F, data_len);
        for (int i = 0; i < data_len; i++)
        {
            write_reg(0x3F, buffer[i]);
        }
        send_cmd(Commands::STX);
        MISO::SetInterrupt(InterruptMode::FallingEdge); // enable MISO IRQ
        MISO::GetIsrDelegate()
            .template init<CC1101<MOSI, MISO, SCLK, CS>, &CC1101<MOSI, MISO, SCLK, CS>::tx_complete_isr>(
                this); // activate ISR
        
    }

  private:
    enum class Commands : uint8_t
    {
        SRES = 0x30,    // Chip Reset
        SFSTXON = 0x31, // Enable and calibrate frequency synthesizer
        SXOFF = 0x32,   // Turn Off OSC
        SCAL = 0x33,    // Calibrate frequency synthesizer
        SRX = 0x34,     // Enable RX
        STX = 0x35,     // Enable TX
        SIDLE = 0x36,   // Exit RX/TX, go in idle mode
        SWOR = 0x38,    // Start automatic polling sequence
        SPWD = 0x39,    // Enter power down mode
        SFRX = 0x3A,    // Flush RX FIFO
        SFTX = 0x3B,    // Flush TX FIFO
        SWORRST = 0x3C, // Reset real time clock
        SNOP = 0x3D,    // no operation
        MARCSTATE = 0x35
    };

    std::shared_ptr<SoftwareTimer> timeout_timer;

    void tx_complete_isr()
    {
        MISO::DisableInterrupt();
        send_cmd(Commands::SFTX);
        send_cmd(Commands::SIDLE);
        timeout_timer->stop();
        tx_complete();
        return;
    }

    void rx_complete_isr()
    {
        MISO::DisableInterrupt();
         ;
        int n = read_reg(0x3F);
        int RSSI = 0;
        int LIQ = 0;
        if ((n > 0))
        {
            Buffer buffer(n);
            int data_len = 0;
            while ((n != 0) && (data_len < 64))
            {
                buffer[data_len] = read_reg(0x3F);
                data_len++;
                n--;
            }
            const int r = read_reg(0x3F);
            if (r >= 128)
            {
                // RSSI = (r - 256) / 2.0 - 74;
                RSSI = r - 256 - 148;
            }
            else
            {
                // RSSI = r / 2.0 - 74;
                RSSI = r - 148;
            }
            LIQ = read_reg(0x3F);
            if (LIQ & 0x80)
            {
                //	freqest += ReadReg(0x32);
                //	WriteReg(0x0C, (signed int)(freqest / 100));
            }
            LIQ &= 0x7F;
            send_cmd(Commands::SFRX);
            send_cmd(Commands::SIDLE);
            rx_complete(buffer, RSSI, LIQ);
            return;
        }

        send_cmd(Commands::SIDLE);
    }

    uint8_t send_cmd(Commands cmd)
    {

        CS::Reset();
        while (MISO::Read())
            ;
        uint8_t state = send_byte((uint8_t)cmd);
        CS::Set();
        return state;
    }

    void wait_for_idle()
    {
        for (int n = 0; n < 1000; n++)
        {
            if ((send_cmd(Commands::SIDLE) & 0x70) == 0)
                break;
            Delay_ms(1);
        }
    }

    void rx_timeout_isr()
    {
        MISO::DisableInterrupt(); // Disable IRQ
        wait_for_idle();          // Disable radio
        rx_error();
    }

    void write_reg(uint16_t addr, uint8_t data)
    {
        CS::Reset();
        while (MISO::Read())
            ; //
        const uint8_t header = addr;
        send_byte(header);
        send_byte(data);
        CS::Set();
    }

    uint8_t read_reg(uint16_t addr)
    {
        CS::Reset();
        while (MISO::Read())
            ;
        const uint8_t header = addr | 0x80 | 0x40;
        send_byte(header);
        const auto data = send_byte(0);
        CS::Set();
        return data;
    }

    uint8_t send_byte(uint8_t byte)
    {
        uint8_t r = 0;
        for (int i = 7; i >= 0; i--)
        {
            MOSI::Write(byte & (1 << i));
            r = MISO::Read() ? r | (1 << i) : r;

            Delay_us(1);
            SCLK::Set();

            Delay_us(1);
            SCLK::Reset();

            Delay_us(1);
        }
        return r;
    }
};
