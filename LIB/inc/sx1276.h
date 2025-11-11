#pragma once
#include "SPI_base.h"
#include "delay.h"
#include "delegate.h"
#include "gpio_interface.h"
#include "software_timer_interface.h"
#include <array>
#include <cstring>
#include <stdint.h>
#include <string>

//#include <stdbool.h>
//#include "gpio.h"
//#include "spi.h"
//#include "radio.h"
#include "sx1276Regs-Fsk.h"
#include "sx1276Regs-LoRa.h"

template <typename TIMER, typename RESET_pin, typename NSS_pin, typename DIO0_pin, typename DIO1_pin, typename DIO2_pin,
          typename DIO3_pin, typename DIO4_pin>
class LoraModem
{
  private:
    static constexpr uint32_t SX1276_XTAL_FREQ = 32000000UL;
    static constexpr int32_t SX1276_PLL_STEP_SHIFT_AMOUNT = 8;
    static constexpr int32_t SX1276_PLL_STEP_SCALED = (SX1276_XTAL_FREQ >> (19 - SX1276_PLL_STEP_SHIFT_AMOUNT));
    static constexpr uint32_t RX_TX_BUFFER_SIZE = 256;
    static constexpr uint32_t RF_MID_BAND_THRESH = 525000000;
    static constexpr int RSSI_OFFSET_HF = -157;
    static constexpr int RSSI_OFFSET_LF = -164;

  public:
    enum RadioModems_t
    {
        MODEM_FSK = 0,
        MODEM_LORA,
    };

  private:
    enum RadioState_t
    {
        RF_IDLE = 0,
        RF_RX_RUNNING,
        RF_TX_RUNNING,
        RF_CAD,
        RF_LBT
    };

    struct RadioRegisters_t
    {
        RadioModems_t Modem;
        uint8_t Addr;
        uint8_t Value;
    };

    struct RadioFskSettings_t
    {
        int8_t Power;
        uint32_t Fdev;
        uint32_t Bandwidth;
        uint32_t BandwidthAfc;
        uint32_t Datarate;
        uint16_t PreambleLen;
        bool FixLen;
        uint8_t PayloadLen;
        bool CrcOn;
        bool IqInverted;
        bool RxContinuous;
        uint32_t TxTimeout;
        uint32_t RxSingleTimeout;
    };
    struct RadioFskPacketHandler_t
    {
        uint8_t PreambleDetected;
        uint8_t SyncWordDetected;
        int8_t RssiValue;
        int32_t AfcValue;
        uint8_t RxGain;
        uint16_t Size;
        uint16_t NbBytes;
        uint8_t FifoThresh;
        uint8_t ChunkSize;
    };
    struct RadioLoRaSettings_t
    {
        int8_t Power;
        uint32_t Bandwidth;
        uint32_t Datarate;
        bool LowDatarateOptimize;
        uint8_t Coderate;
        uint16_t PreambleLen;
        bool FixLen;
        uint8_t PayloadLen;
        bool CrcOn;
        bool FreqHopOn;
        uint8_t HopPeriod;
        bool IqInverted;
        bool RxContinuous;
        uint32_t TxTimeout;
        bool PublicNetwork;
    };
    struct RadioLoRaPacketHandler_t
    {
        int8_t SnrValue;
        int16_t RssiValue;
        uint8_t Size;
    };
    struct RadioSettings_t
    {
        RadioState_t State;
        RadioModems_t Modem;
        uint32_t Channel;
        RadioFskSettings_t Fsk;
        RadioFskPacketHandler_t FskPacketHandler;
        RadioLoRaSettings_t LoRa;
        RadioLoRaPacketHandler_t LoRaPacketHandler;
    };

    struct FskBandwidth_t
    {
        uint32_t bandwidth;
        uint8_t RegValue;
    };

    constexpr static std::array<RadioRegisters_t, 16> RadioRegsInit = {{{MODEM_FSK, REG_LNA, 0x23},
                                                                        {MODEM_FSK, REG_RXCONFIG, 0x1E},
                                                                        {MODEM_FSK, REG_RSSICONFIG, 0xD2},
                                                                        {MODEM_FSK, REG_AFCFEI, 0x01},
                                                                        {MODEM_FSK, REG_PREAMBLEDETECT, 0xAA},
                                                                        {MODEM_FSK, REG_OSC, 0x07},
                                                                        {MODEM_FSK, REG_SYNCCONFIG, 0x12},
                                                                        {MODEM_FSK, REG_SYNCVALUE1, 0xC1},
                                                                        {MODEM_FSK, REG_SYNCVALUE2, 0x94},
                                                                        {MODEM_FSK, REG_SYNCVALUE3, 0xC1},
                                                                        {MODEM_FSK, REG_PACKETCONFIG1, 0xD8},
                                                                        {MODEM_FSK, REG_FIFOTHRESH, 0x8F},
                                                                        {MODEM_FSK, REG_IMAGECAL, 0x02},
                                                                        {MODEM_FSK, REG_DIOMAPPING1, 0x00},
                                                                        {MODEM_FSK, REG_DIOMAPPING2, 0x30},
                                                                        {MODEM_LORA, REG_LR_PAYLOADMAXLENGTH, 0x40}}};
    constexpr static std::array<FskBandwidth_t, 22> FskBandwidths = {{
        {2600, 0x17},   {3100, 0x0F},   {3900, 0x07},   {5200, 0x16},   {6300, 0x0E},   {7800, 0x06},
        {10400, 0x15},  {12500, 0x0D},  {15600, 0x05},  {20800, 0x14},  {25000, 0x0C},  {31300, 0x04},
        {41700, 0x13},  {50000, 0x0B},  {62500, 0x03},  {83333, 0x12},  {100000, 0x0A}, {125000, 0x02},
        {166700, 0x11}, {200000, 0x09}, {250000, 0x01}, {300000, 0x00}, // Invalid Bandwidth
    }};

  public:
    LoraModem(SPI_MasterBase &spi) : _spi{&spi}
    {
    }

    void init()
    {
        uint8_t i;
        GpioInit();
        // RadioEvents = events; события представлены делегатами в этом классе

        // Initialize driver timeout timers
        /*
        TimerInit(&TxTimeoutTimer, SX1276OnTimeoutIrq);
        TimerInit(&RxTimeoutTimer, SX1276OnTimeoutIrq);
        TimerInit(&RxTimeoutSyncWord, SX1276OnTimeoutIrq);
*/
        TxTimeoutTimer.init(10);
        TxTimeoutTimer.tick.template init<LoraModem, &LoraModem::SX1276OnTimeoutIrq>(this);
        RxTimeoutTimer.init(10);
        RxTimeoutTimer.tick.template init<LoraModem, &LoraModem::SX1276OnTimeoutIrq>(this);
        RxTimeoutSyncWord.init(10);
        RxTimeoutSyncWord.tick.template init<LoraModem, &LoraModem::SX1276OnTimeoutIrq>(this);
        SX1276Reset();

        RxChainCalibration();

        SX1276SetOpMode(RF_OPMODE_SLEEP);

        SX1276IoIrqInit();

        for (i = 0; i < RadioRegsInit.size(); i++)
        {
            SX1276SetModem(RadioRegsInit[i].Modem);
            SX1276Write(RadioRegsInit[i].Addr, RadioRegsInit[i].Value);
        }

        SX1276SetModem(MODEM_FSK);

        Settings.State = RF_IDLE;
    }

    void SX1276SetTxContinuousWave(uint32_t freq, int8_t power, uint32_t time)
    {
        // uint32_t timeout = (uint32_t)time * 1000; sec to ms
        uint32_t timeout = time;
        SX1276SetChannel(freq);

        SX1276SetTxConfig(MODEM_FSK, power, 0, 0, 4800, 0, 5, false, false, 0, 0, 0, timeout);

        SX1276Write(REG_PACKETCONFIG2, (SX1276Read(REG_PACKETCONFIG2) & RF_PACKETCONFIG2_DATAMODE_MASK));
        // Disable radio interrupts
        SX1276Write(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_11 | RF_DIOMAPPING1_DIO1_11);
        SX1276Write(REG_DIOMAPPING2, RF_DIOMAPPING2_DIO4_10 | RF_DIOMAPPING2_DIO5_10);

        // TimerSetValue(&TxTimeoutTimer, timeout);
        TxTimeoutTimer.start_one_shoot(timeout);

        Settings.State = RF_TX_RUNNING;
        // TimerStart(&TxTimeoutTimer);
        TxTimeoutTimer.start_one_shoot(timeout);
        SX1276SetOpMode(RF_OPMODE_TRANSMITTER);
    }

    void SX1276SetTx(uint32_t timeout)
    {
        // TimerStop(&RxTimeoutTimer);
        RxTimeoutTimer.stop();

        // TimerSetValue(&TxTimeoutTimer, timeout);

        switch (Settings.Modem)
        {
        case MODEM_FSK: {
            // DIO0=PacketSent
            // DIO1=FifoLevel
            // DIO2=FifoFull
            // DIO3=FifoEmpty
            // DIO4=LowBat
            // DIO5=ModeReady
            SX1276Write(REG_DIOMAPPING1, (SX1276Read(REG_DIOMAPPING1) & RF_DIOMAPPING1_DIO0_MASK &
                                          RF_DIOMAPPING1_DIO1_MASK & RF_DIOMAPPING1_DIO2_MASK));

            SX1276Write(REG_DIOMAPPING2,
                        (SX1276Read(REG_DIOMAPPING2) & RF_DIOMAPPING2_DIO4_MASK & RF_DIOMAPPING2_MAP_MASK));
            Settings.FskPacketHandler.FifoThresh = SX1276Read(REG_FIFOTHRESH) & 0x3F;
        }
        break;
        case MODEM_LORA: {
            if (Settings.LoRa.FreqHopOn == true)
            {
                SX1276Write(REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT | RFLR_IRQFLAGS_RXDONE |
                                                     RFLR_IRQFLAGS_PAYLOADCRCERROR | RFLR_IRQFLAGS_VALIDHEADER |
                                                     // RFLR_IRQFLAGS_TXDONE |
                                                     RFLR_IRQFLAGS_CADDONE |
                                                     // RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                                     RFLR_IRQFLAGS_CADDETECTED);

                // DIO0=TxDone, DIO2=FhssChangeChannel
                SX1276Write(REG_DIOMAPPING1,
                            (SX1276Read(REG_DIOMAPPING1) & RFLR_DIOMAPPING1_DIO0_MASK & RFLR_DIOMAPPING1_DIO2_MASK) |
                                RFLR_DIOMAPPING1_DIO0_01 | RFLR_DIOMAPPING1_DIO2_00);
            }
            else
            {
                SX1276Write(REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT | RFLR_IRQFLAGS_RXDONE |
                                                     RFLR_IRQFLAGS_PAYLOADCRCERROR | RFLR_IRQFLAGS_VALIDHEADER |
                                                     // RFLR_IRQFLAGS_TXDONE |
                                                     RFLR_IRQFLAGS_CADDONE | RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                                     RFLR_IRQFLAGS_CADDETECTED);

                // DIO0=TxDone
                SX1276Write(REG_DIOMAPPING1,
                            (SX1276Read(REG_DIOMAPPING1) & RFLR_DIOMAPPING1_DIO0_MASK) | RFLR_DIOMAPPING1_DIO0_01);
            }
        }
        break;
        }

        Settings.State = RF_TX_RUNNING;
        // TimerStart(&TxTimeoutTimer);
        TxTimeoutTimer.start_one_shoot(timeout);
        SX1276SetOpMode(RF_OPMODE_TRANSMITTER);
    }
    /*!
     * \brief Sets the transmission parameters
     *
     * \remark When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
     *
     * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
     * \param [IN] power        Sets the output power [dBm]
     * \param [IN] fdev         Sets the frequency deviation (FSK only)
     *                          FSK : [Hz]
     *                          LoRa: 0
     * \param [IN] bandwidth    Sets the bandwidth (LoRa only)
     *                          FSK : 0
     *                          LoRa: [0: 7.8 kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2kHz,
     *                                 5: 41.7 kHz, 6: 62.5 kHz, 7: 125kHz, 8: 250 kHz, 9: 500kHz]
     * \param [IN] datarate     Sets the Datarate
     *                          FSK : 600..300000 bits/s
     *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
     *                                10: 1024, 11: 2048, 12: 4096  chips]
     * \param [IN] coderate     Sets the coding rate (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
     * \param [IN] preambleLen  Sets the preamble length
     *                          FSK : Number of bytes
     *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
     * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
     * \param [IN] crcOn        Enables disables the CRC [0: OFF, 1: ON]
     * \param [IN] freqHopOn    Enables disables the intra-packet frequency hopping
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: OFF, 1: ON]
     * \param [IN] hopPeriod    Number of symbols between each hop
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: Number of symbols
     * \param [IN] iqInverted   Inverts IQ signals (LoRa only)
     *                          FSK : N/A ( set to 0 )
     *                          LoRa: [0: not inverted, 1: inverted]
     * \param [IN] timeout      Transmission timeout [ms]
     */
    void SX1276SetTxConfig(RadioModems_t modem, int8_t power, uint32_t fdev, uint32_t bandwidth, uint32_t datarate,
                           uint8_t coderate, uint16_t preambleLen, bool fixLen, bool crcOn, bool freqHopOn,
                           uint8_t hopPeriod, bool iqInverted, uint32_t timeout)
    {
        SX1276SetModem(modem);

        SX1276SetRfTxPower(power);

        switch (modem)
        {
        case MODEM_FSK: {
            Settings.Fsk.Power = power;
            Settings.Fsk.Fdev = fdev;
            Settings.Fsk.Bandwidth = bandwidth;
            Settings.Fsk.Datarate = datarate;
            Settings.Fsk.PreambleLen = preambleLen;
            Settings.Fsk.FixLen = fixLen;
            Settings.Fsk.CrcOn = crcOn;
            Settings.Fsk.IqInverted = iqInverted;
            Settings.Fsk.TxTimeout = timeout;

            uint32_t fdevInPllSteps = SX1276ConvertFreqInHzToPllStep(fdev);
            SX1276Write(REG_FDEVMSB, (uint8_t)(fdevInPllSteps >> 8));
            SX1276Write(REG_FDEVLSB, (uint8_t)(fdevInPllSteps & 0xFF));

            uint32_t bitRate = (uint32_t)(SX1276_XTAL_FREQ / datarate);
            SX1276Write(REG_BITRATEMSB, (uint8_t)(bitRate >> 8));
            SX1276Write(REG_BITRATELSB, (uint8_t)(bitRate & 0xFF));

            SX1276Write(REG_PREAMBLEMSB, (preambleLen >> 8) & 0x00FF);
            SX1276Write(REG_PREAMBLELSB, preambleLen & 0xFF);

            SX1276Write(
                REG_PACKETCONFIG1,
                (SX1276Read(REG_PACKETCONFIG1) & RF_PACKETCONFIG1_CRC_MASK & RF_PACKETCONFIG1_PACKETFORMAT_MASK) |
                    ((fixLen == 1) ? RF_PACKETCONFIG1_PACKETFORMAT_FIXED : RF_PACKETCONFIG1_PACKETFORMAT_VARIABLE) |
                    (crcOn << 4));
            SX1276Write(REG_PACKETCONFIG2, (SX1276Read(REG_PACKETCONFIG2) | RF_PACKETCONFIG2_DATAMODE_PACKET));
        }
        break;
        case MODEM_LORA: {
            Settings.LoRa.Power = power;
            if (bandwidth > 9)
            {
                // Fatal error: When using LoRa modem only to 500 kHz are supported
                while (1)
                    ;
            }
            bandwidth += 0;
            Settings.LoRa.Bandwidth = bandwidth;
            Settings.LoRa.Datarate = datarate;
            Settings.LoRa.Coderate = coderate;
            Settings.LoRa.PreambleLen = preambleLen;
            Settings.LoRa.FixLen = fixLen;
            Settings.LoRa.FreqHopOn = freqHopOn;
            Settings.LoRa.HopPeriod = hopPeriod;
            Settings.LoRa.CrcOn = crcOn;
            Settings.LoRa.IqInverted = iqInverted;
            Settings.LoRa.TxTimeout = timeout;

            if (datarate > 12)
            {
                datarate = 12;
            }
            else if (datarate < 6)
            {
                datarate = 6;
            }
            if (((bandwidth == 7) && ((datarate == 11) || (datarate == 12))) || ((bandwidth == 8) && (datarate == 12)))
            {
                Settings.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                Settings.LoRa.LowDatarateOptimize = 0x00;
            }

            if (Settings.LoRa.FreqHopOn == true)
            {
                SX1276Write(REG_LR_PLLHOP,
                            (SX1276Read(REG_LR_PLLHOP) & RFLR_PLLHOP_FASTHOP_MASK) | RFLR_PLLHOP_FASTHOP_ON);
                SX1276Write(REG_LR_HOPPERIOD, Settings.LoRa.HopPeriod);
            }

            SX1276Write(REG_LR_MODEMCONFIG1,
                        (SX1276Read(REG_LR_MODEMCONFIG1) & RFLR_MODEMCONFIG1_BW_MASK &
                         RFLR_MODEMCONFIG1_CODINGRATE_MASK & RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK) |
                            (bandwidth << 4) | (coderate << 1) | fixLen);

            SX1276Write(REG_LR_MODEMCONFIG2, (SX1276Read(REG_LR_MODEMCONFIG2) & RFLR_MODEMCONFIG2_SF_MASK &
                                              RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK) |
                                                 (datarate << 4) | (crcOn << 2));

            SX1276Write(REG_LR_MODEMCONFIG3,
                        (SX1276Read(REG_LR_MODEMCONFIG3) & RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK) |
                            (Settings.LoRa.LowDatarateOptimize << 3));

            SX1276Write(REG_LR_PREAMBLEMSB, (preambleLen >> 8) & 0x00FF);
            SX1276Write(REG_LR_PREAMBLELSB, preambleLen & 0xFF);

            if (datarate == 6)
            {
                SX1276Write(REG_LR_DETECTOPTIMIZE, (SX1276Read(REG_LR_DETECTOPTIMIZE) & RFLR_DETECTIONOPTIMIZE_MASK) |
                                                       RFLR_DETECTIONOPTIMIZE_SF6);
                SX1276Write(REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF6);
            }
            else
            {
                SX1276Write(REG_LR_DETECTOPTIMIZE, (SX1276Read(REG_LR_DETECTOPTIMIZE) & RFLR_DETECTIONOPTIMIZE_MASK) |
                                                       RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12);
                SX1276Write(REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12);
            }
        }
        break;
        }
    }

    void SX1276Send(uint8_t *buffer, uint8_t size)
    {
        uint32_t txTimeout = 0;

        switch (Settings.Modem)
        {
        case MODEM_FSK: {
            Settings.FskPacketHandler.NbBytes = 0;
            Settings.FskPacketHandler.Size = size;

            if (Settings.Fsk.FixLen == false)
            {
                SX1276WriteFifo((uint8_t *)&size, 1);
            }
            else
            {
                SX1276Write(REG_PAYLOADLENGTH, size);
            }

            if ((size > 0) && (size <= 64))
            {
                Settings.FskPacketHandler.ChunkSize = size;
            }
            else
            {
                std::memcpy(RxTxBuffer, buffer, size);
                Settings.FskPacketHandler.ChunkSize = 32;
            }

            // Write payload buffer
            SX1276WriteFifo(buffer, Settings.FskPacketHandler.ChunkSize);
            Settings.FskPacketHandler.NbBytes += Settings.FskPacketHandler.ChunkSize;
            txTimeout = Settings.Fsk.TxTimeout;
        }
        break;
        case MODEM_LORA: {
            if (Settings.LoRa.IqInverted == true)
            {
                SX1276Write(REG_LR_INVERTIQ,
                            ((SX1276Read(REG_LR_INVERTIQ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK) |
                             RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_ON));
                SX1276Write(REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON);
            }
            else
            {
                SX1276Write(REG_LR_INVERTIQ,
                            ((SX1276Read(REG_LR_INVERTIQ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK) |
                             RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF));
                SX1276Write(REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF);
            }

            Settings.LoRaPacketHandler.Size = size;

            // Initializes the payload size
            SX1276Write(REG_LR_PAYLOADLENGTH, size);

            // Full buffer used for Tx
            SX1276Write(REG_LR_FIFOTXBASEADDR, 0);
            SX1276Write(REG_LR_FIFOADDRPTR, 0);

            // FIFO operations can not take place in Sleep mode
            if ((SX1276Read(REG_OPMODE) & ~RF_OPMODE_MASK) == RF_OPMODE_SLEEP)
            {
                SX1276SetStby();
                // DelayMs(1);
                Delay_ms(1);
            }
            // Write payload buffer
            SX1276WriteFifo(buffer, size);
            txTimeout = Settings.LoRa.TxTimeout;
        }
        break;
        }

        SX1276SetTx(txTimeout);
    }

    void SX1276SetStby(void)
    {
        // TimerStop(&RxTimeoutTimer);
        RxTimeoutTimer.stop();
        // TimerStop(&TxTimeoutTimer);
        TxTimeoutTimer.stop();
        // TimerStop(&RxTimeoutSyncWord);
        RxTimeoutSyncWord.stop();

        SX1276SetOpMode(RF_OPMODE_STANDBY);
        Settings.State = RF_IDLE;
    }

    void SX1276SetChannel(uint32_t freq)
    {
        uint32_t freqInPllSteps = SX1276ConvertFreqInHzToPllStep(freq);

        Settings.Channel = freq;

        SX1276Write(REG_FRFMSB, (uint8_t)((freqInPllSteps >> 16) & 0xFF));
        SX1276Write(REG_FRFMID, (uint8_t)((freqInPllSteps >> 8) & 0xFF));
        SX1276Write(REG_FRFLSB, (uint8_t)(freqInPllSteps & 0xFF));
    }

    void SX1276SetSleep(void)
    {
        // TimerStop(&RxTimeoutTimer);
        RxTimeoutTimer.stop();
        // TimerStop(&TxTimeoutTimer);
        TxTimeoutTimer.stop();
        // TimerStop(&RxTimeoutSyncWord);
        RxTimeoutSyncWord.stop();

        SX1276SetOpMode(RF_OPMODE_SLEEP);

        // Disable TCXO radio is in SLEEP mode
        // TODO:SX1276SetBoardTcxo( false );

        Settings.State = RF_IDLE;
    }

    void SX1276SetOpMode(uint8_t opMode)
    {
        if (opMode == RF_OPMODE_SLEEP)
        {
            // TODO: SX1276SetAntSwLowPower
            // SX1276SetAntSwLowPower( true );
        }
        else
        {
            // Enable TCXO if operating mode different from SLEEP.
            // TODO: SX1276SetBoardTcxo( true );
            // TODO: SX1276SetAntSwLowPower( false );
            // TODO: SX1276SetAntSw( opMode );
        }
        SX1276Write(REG_OPMODE, (SX1276Read(REG_OPMODE) & RF_OPMODE_MASK) | opMode);
    }
///*!
// * \brief Sets the reception parameters
// *
// * \remark When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
// *
// * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
// * \param [IN] bandwidth    Sets the bandwidth
// *                          FSK : >= 2600 and <= 250000 Hz
// *                          LoRa: [0: 125 kHz, 1: 250 kHz,
// *                                 2: 500 kHz, 3: Reserved]
// * \param [IN] datarate     Sets the Datarate
// *                          FSK : 600..300000 bits/s
// *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
// *                                10: 1024, 11: 2048, 12: 4096  chips]
// * \param [IN] coderate     Sets the coding rate (LoRa only)
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
// * \param [IN] bandwidthAfc Sets the AFC Bandwidth (FSK only)
// *                          FSK : >= 2600 and <= 250000 Hz
// *                          LoRa: N/A ( set to 0 )
// * \param [IN] preambleLen  Sets the Preamble length
// *                          FSK : Number of bytes
// *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
// * \param [IN] symbTimeout  Sets the RxSingle timeout value
// *                          FSK : timeout number of bytes
// *                          LoRa: timeout in symbols
// * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
// * \param [IN] payloadLen   Sets payload length when fixed length is used
// * \param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
// * \param [IN] freqHopOn    Enables disables the intra-packet frequency hopping
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [0: OFF, 1: ON]
// * \param [IN] hopPeriod    Number of symbols between each hop
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: Number of symbols
// * \param [IN] iqInverted   Inverts IQ signals (LoRa only)
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [0: not inverted, 1: inverted]
// * \param [IN] rxContinuous Sets the reception in continuous mode
// *                          [false: single mode, true: continuous mode]
// */
    void SX1276SetRxConfig(RadioModems_t modem, uint32_t bandwidth, uint32_t datarate, uint8_t coderate,
                           uint32_t bandwidthAfc, uint16_t preambleLen, uint16_t symbTimeout, bool fixLen,
                           uint8_t payloadLen, bool crcOn, bool freqHopOn, uint8_t hopPeriod, bool iqInverted,
                           bool rxContinuous)
    { // TODO: проверить
        SX1276SetModem(modem);

        switch (modem)
        {
        case MODEM_FSK: {
            Settings.Fsk.Bandwidth = bandwidth;
            Settings.Fsk.Datarate = datarate;
            Settings.Fsk.BandwidthAfc = bandwidthAfc;
            Settings.Fsk.FixLen = fixLen;
            Settings.Fsk.PayloadLen = payloadLen;
            Settings.Fsk.CrcOn = crcOn;
            Settings.Fsk.IqInverted = iqInverted;
            Settings.Fsk.RxContinuous = rxContinuous;
            Settings.Fsk.PreambleLen = preambleLen;
            Settings.Fsk.RxSingleTimeout = (uint32_t)symbTimeout * 8000UL / datarate;

            uint32_t bitRate = (uint32_t)(SX1276_XTAL_FREQ / datarate);
            SX1276Write(REG_BITRATEMSB, (uint8_t)(bitRate >> 8));
            SX1276Write(REG_BITRATELSB, (uint8_t)(bitRate & 0xFF));

            SX1276Write(REG_RXBW, GetFskBandwidthRegValue(bandwidth));
            SX1276Write(REG_AFCBW, GetFskBandwidthRegValue(bandwidthAfc));

            SX1276Write(REG_PREAMBLEMSB, (uint8_t)((preambleLen >> 8) & 0xFF));
            SX1276Write(REG_PREAMBLELSB, (uint8_t)(preambleLen & 0xFF));

            if (fixLen == 1)
            {
                SX1276Write(REG_PAYLOADLENGTH, payloadLen);
            }
            else
            {
                SX1276Write(REG_PAYLOADLENGTH, 0xFF); // Set payload length to the maximum
            }

            SX1276Write(
                REG_PACKETCONFIG1,
                (SX1276Read(REG_PACKETCONFIG1) & RF_PACKETCONFIG1_CRC_MASK & RF_PACKETCONFIG1_PACKETFORMAT_MASK) |
                    ((fixLen == 1) ? RF_PACKETCONFIG1_PACKETFORMAT_FIXED : RF_PACKETCONFIG1_PACKETFORMAT_VARIABLE) |
                    (crcOn << 4));
            SX1276Write(REG_PACKETCONFIG2, (SX1276Read(REG_PACKETCONFIG2) | RF_PACKETCONFIG2_DATAMODE_PACKET));
        }
        break;
        case MODEM_LORA: {
            // if (bandwidth > 2)
            //{
            //  Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
            //    while (1)
            //        ;
            //}
            // bandwidth += 7;
            Settings.LoRa.Bandwidth = bandwidth;
            Settings.LoRa.Datarate = datarate;
            Settings.LoRa.Coderate = coderate;
            Settings.LoRa.PreambleLen = preambleLen;
            Settings.LoRa.FixLen = fixLen;
            Settings.LoRa.PayloadLen = payloadLen;
            Settings.LoRa.CrcOn = crcOn;
            Settings.LoRa.FreqHopOn = freqHopOn;
            Settings.LoRa.HopPeriod = hopPeriod;
            Settings.LoRa.IqInverted = iqInverted;
            Settings.LoRa.RxContinuous = rxContinuous;

            if (datarate > 12)
            {
                datarate = 12;
            }
            else if (datarate < 6)
            {
                datarate = 6;
            }

            if (((bandwidth == 7) && ((datarate == 11) || (datarate == 12))) || ((bandwidth == 8) && (datarate == 12)))
            {
                Settings.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                Settings.LoRa.LowDatarateOptimize = 0x00;
            }

            SX1276Write(REG_LR_MODEMCONFIG1,
                        (SX1276Read(REG_LR_MODEMCONFIG1) & RFLR_MODEMCONFIG1_BW_MASK &
                         RFLR_MODEMCONFIG1_CODINGRATE_MASK & RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK) |
                            (bandwidth << 4) | (coderate << 1) | fixLen);

            SX1276Write(REG_LR_MODEMCONFIG2,
                        (SX1276Read(REG_LR_MODEMCONFIG2) & RFLR_MODEMCONFIG2_SF_MASK &
                         RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK & RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK) |
                            (datarate << 4) | (crcOn << 2) |
                            ((symbTimeout >> 8) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK));

            SX1276Write(REG_LR_MODEMCONFIG3,
                        (SX1276Read(REG_LR_MODEMCONFIG3) & RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK) |
                            (Settings.LoRa.LowDatarateOptimize << 3));

            SX1276Write(REG_LR_SYMBTIMEOUTLSB, (uint8_t)(symbTimeout & 0xFF));

            SX1276Write(REG_LR_PREAMBLEMSB, (uint8_t)((preambleLen >> 8) & 0xFF));
            SX1276Write(REG_LR_PREAMBLELSB, (uint8_t)(preambleLen & 0xFF));

            if (fixLen == 1)
            {
                SX1276Write(REG_LR_PAYLOADLENGTH, payloadLen);
            }

            if (Settings.LoRa.FreqHopOn == true)
            {
                SX1276Write(REG_LR_PLLHOP,
                            (SX1276Read(REG_LR_PLLHOP) & RFLR_PLLHOP_FASTHOP_MASK) | RFLR_PLLHOP_FASTHOP_ON);
                SX1276Write(REG_LR_HOPPERIOD, Settings.LoRa.HopPeriod);
            }

            if ((bandwidth == 9) && (Settings.Channel > RF_MID_BAND_THRESH))
            {
                // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
                SX1276Write(REG_LR_HIGHBWOPTIMIZE1, 0x02);
                SX1276Write(REG_LR_HIGHBWOPTIMIZE2, 0x64);
            }
            else if (bandwidth == 9)
            {
                // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
                SX1276Write(REG_LR_HIGHBWOPTIMIZE1, 0x02);
                SX1276Write(REG_LR_HIGHBWOPTIMIZE2, 0x7F);
            }
            else
            {
                // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
                SX1276Write(REG_LR_HIGHBWOPTIMIZE1, 0x03);
            }

            if (datarate == 6)
            {
                SX1276Write(REG_LR_DETECTOPTIMIZE, (SX1276Read(REG_LR_DETECTOPTIMIZE) & RFLR_DETECTIONOPTIMIZE_MASK) |
                                                       RFLR_DETECTIONOPTIMIZE_SF6);
                SX1276Write(REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF6);
            }
            else
            {
                SX1276Write(REG_LR_DETECTOPTIMIZE, (SX1276Read(REG_LR_DETECTOPTIMIZE) & RFLR_DETECTIONOPTIMIZE_MASK) |
                                                       RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12);
                SX1276Write(REG_LR_DETECTIONTHRESHOLD, RFLR_DETECTIONTHRESH_SF7_TO_SF12);
            }
        }
        break;
        }
    }

    void SX1276SetRx(uint32_t timeout)
    {
        bool rxContinuous = false;
        // TimerStop(&TxTimeoutTimer);
        TxTimeoutTimer.stop();
        switch (Settings.Modem)
        {
        case MODEM_FSK: {
            rxContinuous = Settings.Fsk.RxContinuous;

            // DIO0=PayloadReady
            // DIO1=FifoLevel
            // DIO2=SyncAddr
            // DIO3=FifoEmpty
            // DIO4=Preamble
            // DIO5=ModeReady
            SX1276Write(REG_DIOMAPPING1, (SX1276Read(REG_DIOMAPPING1) & RF_DIOMAPPING1_DIO0_MASK &
                                          RF_DIOMAPPING1_DIO1_MASK & RF_DIOMAPPING1_DIO2_MASK) |
                                             RF_DIOMAPPING1_DIO0_00 | RF_DIOMAPPING1_DIO1_00 | RF_DIOMAPPING1_DIO2_11);

            SX1276Write(REG_DIOMAPPING2,
                        (SX1276Read(REG_DIOMAPPING2) & RF_DIOMAPPING2_DIO4_MASK & RF_DIOMAPPING2_MAP_MASK) |
                            RF_DIOMAPPING2_DIO4_11 | RF_DIOMAPPING2_MAP_PREAMBLEDETECT);

            Settings.FskPacketHandler.FifoThresh = SX1276Read(REG_FIFOTHRESH) & 0x3F;

            SX1276Write(REG_RXCONFIG,
                        RF_RXCONFIG_AFCAUTO_ON | RF_RXCONFIG_AGCAUTO_ON | RF_RXCONFIG_RXTRIGER_PREAMBLEDETECT);

            Settings.FskPacketHandler.PreambleDetected = false;
            Settings.FskPacketHandler.SyncWordDetected = false;
            Settings.FskPacketHandler.NbBytes = 0;
            Settings.FskPacketHandler.Size = 0;
        }
        break;
        case MODEM_LORA: {
            if (Settings.LoRa.IqInverted == true)
            {
                SX1276Write(REG_LR_INVERTIQ,
                            ((SX1276Read(REG_LR_INVERTIQ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK) |
                             RFLR_INVERTIQ_RX_ON | RFLR_INVERTIQ_TX_OFF));
                SX1276Write(REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON);
            }
            else
            {
                SX1276Write(REG_LR_INVERTIQ,
                            ((SX1276Read(REG_LR_INVERTIQ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK) |
                             RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF));
                SX1276Write(REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF);
            }

            // ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
            if (Settings.LoRa.Bandwidth < 9)
            {
                SX1276Write(REG_LR_DETECTOPTIMIZE, SX1276Read(REG_LR_DETECTOPTIMIZE) & 0x7F);
                SX1276Write(REG_LR_IFFREQ2, 0x00);
                switch (Settings.LoRa.Bandwidth)
                {
                case 0: // 7.8 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x48);
                    SX1276SetChannel(Settings.Channel + 7810);
                    break;
                case 1: // 10.4 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x44);
                    SX1276SetChannel(Settings.Channel + 10420);
                    break;
                case 2: // 15.6 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x44);
                    SX1276SetChannel(Settings.Channel + 15620);
                    break;
                case 3: // 20.8 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x44);
                    SX1276SetChannel(Settings.Channel + 20830);
                    break;
                case 4: // 31.2 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x44);
                    SX1276SetChannel(Settings.Channel + 31250);
                    break;
                case 5: // 41.4 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x44);
                    SX1276SetChannel(Settings.Channel + 41670);
                    break;
                case 6: // 62.5 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x40);
                    break;
                case 7: // 125 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x40);
                    break;
                case 8: // 250 kHz
                    SX1276Write(REG_LR_IFFREQ1, 0x40);
                    break;
                }
            }
            else
            {
                SX1276Write(REG_LR_DETECTOPTIMIZE, SX1276Read(REG_LR_DETECTOPTIMIZE) | 0x80);
            }

            rxContinuous = Settings.LoRa.RxContinuous;

            if (Settings.LoRa.FreqHopOn == true)
            {
                SX1276Write(REG_LR_IRQFLAGSMASK, // RFLR_IRQFLAGS_RXTIMEOUT |
                                                 // RFLR_IRQFLAGS_RXDONE |
                                                 // RFLR_IRQFLAGS_PAYLOADCRCERROR |
                            RFLR_IRQFLAGS_VALIDHEADER | RFLR_IRQFLAGS_TXDONE | RFLR_IRQFLAGS_CADDONE |
                                // RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                RFLR_IRQFLAGS_CADDETECTED);

                // DIO0=RxDone, DIO2=FhssChangeChannel
                SX1276Write(REG_DIOMAPPING1,
                            (SX1276Read(REG_DIOMAPPING1) & RFLR_DIOMAPPING1_DIO0_MASK & RFLR_DIOMAPPING1_DIO2_MASK) |
                                RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO2_00);
            }
            else
            {
                SX1276Write(REG_LR_IRQFLAGSMASK, // RFLR_IRQFLAGS_RXTIMEOUT |
                                                 // RFLR_IRQFLAGS_RXDONE |
                                                 // RFLR_IRQFLAGS_PAYLOADCRCERROR |
                            RFLR_IRQFLAGS_VALIDHEADER | RFLR_IRQFLAGS_TXDONE | RFLR_IRQFLAGS_CADDONE |
                                RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL | RFLR_IRQFLAGS_CADDETECTED);

                // DIO0=RxDone
                SX1276Write(REG_DIOMAPPING1,
                            (SX1276Read(REG_DIOMAPPING1) & RFLR_DIOMAPPING1_DIO0_MASK) | RFLR_DIOMAPPING1_DIO0_00);
            }
            SX1276Write(REG_LR_FIFORXBASEADDR, 0);
            SX1276Write(REG_LR_FIFOADDRPTR, 0);
        }
        break;
        }
        std::memset(RxTxBuffer, 0, (size_t)RX_TX_BUFFER_SIZE);

        Settings.State = RF_RX_RUNNING;
        if (timeout != 0)
        {
            //    TimerSetValue(&RxTimeoutTimer, timeout);
            //    TimerStart(&RxTimeoutTimer);
            RxTimeoutTimer.start_one_shoot(timeout);
        }

        if (Settings.Modem == MODEM_FSK)
        {
            SX1276SetOpMode(RF_OPMODE_RECEIVER);

            if (rxContinuous == false)
            {
                // TimerSetValue(&RxTimeoutSyncWord, Settings.Fsk.RxSingleTimeout);
                // TimerStart(&RxTimeoutSyncWord);
                RxTimeoutSyncWord.start_one_shoot(Settings.Fsk.RxSingleTimeout);
            }
        }
        else
        {
            if (rxContinuous == true)
            {
                SX1276SetOpMode(RFLR_OPMODE_RECEIVER);
            }
            else
            {
                SX1276SetOpMode(RFLR_OPMODE_RECEIVER_SINGLE);
            }
        }
    }

    int16_t SX1276ReadRssi(RadioModems_t modem)
    {
        int16_t rssi = 0;

        switch (modem)
        {
        case MODEM_FSK:
            rssi = -(SX1276Read(REG_RSSIVALUE) >> 1);
            break;
        case MODEM_LORA:
            if (Settings.Channel > RF_MID_BAND_THRESH)
            {
                rssi = RSSI_OFFSET_HF + SX1276Read(REG_LR_RSSIVALUE);
            }
            else
            {
                rssi = RSSI_OFFSET_LF + SX1276Read(REG_LR_RSSIVALUE);
            }
            break;
        default:
            rssi = -1;
            break;
        }
        return rssi;
    }

  private:
    void GpioInit()
    {
        RESET_pin::Init();
        RESET_pin::SetMode(Mode::Output);
        NSS_pin::Init();
        NSS_pin::SetMode(Mode::Output, true);
        DIO0_pin::Init();
        DIO0_pin::SetMode(Mode::Input);
        DIO1_pin::Init();
        DIO1_pin::SetMode(Mode::Input);
        DIO2_pin::Init();
        DIO2_pin::SetMode(Mode::Input);
        DIO3_pin::Init();
        DIO3_pin::SetMode(Mode::Input);
        DIO4_pin::Init();
        DIO4_pin::SetMode(Mode::Input);
    }

    void SX1276OnTimeoutIrq()
    {
        switch (Settings.State)
        {
        case RF_RX_RUNNING:
            if (Settings.Modem == MODEM_FSK)
            {
                Settings.FskPacketHandler.PreambleDetected = false;
                Settings.FskPacketHandler.SyncWordDetected = false;
                Settings.FskPacketHandler.NbBytes = 0;
                Settings.FskPacketHandler.Size = 0;

                // Clear Irqs
                SX1276Write(REG_IRQFLAGS1,
                            RF_IRQFLAGS1_RSSI | RF_IRQFLAGS1_PREAMBLEDETECT | RF_IRQFLAGS1_SYNCADDRESSMATCH);
                SX1276Write(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);

                if (Settings.Fsk.RxContinuous == true)
                {
                    // Continuous mode restart Rx chain
                    SX1276Write(REG_RXCONFIG, SX1276Read(REG_RXCONFIG) | RF_RXCONFIG_RESTARTRXWITHOUTPLLLOCK);
                }
                else
                {
                    Settings.State = RF_IDLE;
                    // TimerStop( &RxTimeoutSyncWord );
                    RxTimeoutSyncWord.stop();
                }
            }
            /*
            if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
            {
                RadioEvents->RxTimeout( );
            }
            */
            RxTimeoutEvent();
            break;
        case RF_TX_RUNNING:
            // Tx timeout shouldn't happen.
            // Reported issue of SPI data corruption resulting in TX TIMEOUT
            // is NOT related to a bug in radio transceiver.
            // It is mainly caused by improper PCB routing of SPI lines and/or
            // violation of SPI specifications.
            // To mitigate redesign, Semtech offers a workaround which resets
            // the radio transceiver and putting it into a known state.

            // BEGIN WORKAROUND

            // Reset the radio
            SX1276Reset();

            // Calibrate Rx chain
            RxChainCalibration();

            // Initialize radio default values
            SX1276SetOpMode(RF_OPMODE_SLEEP);
            /*
                        for (uint8_t i = 0; i < sizeof(RadioRegsInit) / sizeof(RadioRegisters_t); i++)
                        {
                            SX1276SetModem(RadioRegsInit[i].Modem);
                            SX1276Write(RadioRegsInit[i].Addr, RadioRegsInit[i].Value);
                        }
                        */
            for (size_t i = 0; i < RadioRegsInit.size(); i++)
            {
                SX1276SetModem(RadioRegsInit[i].Modem);
                SX1276Write(RadioRegsInit[i].Addr, RadioRegsInit[i].Value);
            }
            SX1276SetModem(MODEM_FSK);

            // Restore previous network type setting.
            // SX1276SetPublicNetwork(Settings.LoRa.PublicNetwork); TODO: ?? public network
            // END WORKAROUND

            Settings.State = RF_IDLE;
            /*
            if ((RadioEvents != NULL) && (RadioEvents->TxTimeout != NULL))
            {
                RadioEvents->TxTimeout();
            }
            */
            break;
        default:
            break;
        }
    }

    void SX1276Reset()
    {
        // Set RESET pin to 0
        RESET_pin::SetMode(Mode::Output, false);
        // Wait 1 ms
        Delay_ms(2);
        RESET_pin::Set();
        // Configure RESET as input
        // GpioInit( &SX1276.Reset, RADIO_RESET, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

        // Wait 6 ms
        Delay_ms(6);
    }

    void RxChainCalibration()
    {

        uint8_t regPaConfigInitVal;
        uint32_t initialFreq;

        // Save context
        regPaConfigInitVal = SX1276Read(REG_PACONFIG);

        initialFreq = SX1276ConvertPllStepToFreqInHz((((uint32_t)SX1276Read(REG_FRFMSB) << 16) |
                                                      ((uint32_t)SX1276Read(REG_FRFMID) << 8) |
                                                      ((uint32_t)SX1276Read(REG_FRFLSB))));

        // Cut the PA just in case, RFO output, power = -1 dBm
        SX1276Write(REG_PACONFIG, 0x00);

        // Launch Rx chain calibration for LF band
        SX1276Write(REG_IMAGECAL, (SX1276Read(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_MASK) | RF_IMAGECAL_IMAGECAL_START);
        while ((SX1276Read(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_RUNNING) == RF_IMAGECAL_IMAGECAL_RUNNING)
        {
            Delay_ms(10); // yeled
        }

        // Sets a Frequency in HF band
        SX1276SetChannel(868000000);

        // Launch Rx chain calibration for HF band
        SX1276Write(REG_IMAGECAL, (SX1276Read(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_MASK) | RF_IMAGECAL_IMAGECAL_START);
        while ((SX1276Read(REG_IMAGECAL) & RF_IMAGECAL_IMAGECAL_RUNNING) == RF_IMAGECAL_IMAGECAL_RUNNING)
        {
        }

        // Restore context
        SX1276Write(REG_PACONFIG, regPaConfigInitVal);
        SX1276SetChannel(initialFreq);
    }

    void SX1276IoIrqInit()
    {
        // GpioSetInterrupt( &SX1276.DIO0, IRQ_RISING_EDGE, IRQ_LOW_PRIORITY, irqHandlers[0] );
        DIO0_pin::SetInterrupt(InterruptMode::RisingEdge);
        DIO0_pin::GetIsrDelegate().template init<LoraModem, &LoraModem::SX1276OnDio0Irq>(this);
        // GpioSetInterrupt( &SX1276.DIO1, IRQ_RISING_FALLING_EDGE, IRQ_LOW_PRIORITY, irqHandlers[1] );
        DIO1_pin::SetInterrupt(InterruptMode::ChangeLevel);
        DIO1_pin::GetIsrDelegate().template init<LoraModem, &LoraModem::SX1276OnDio1Irq>(this);
        // GpioSetInterrupt( &SX1276.DIO2, IRQ_RISING_EDGE, IRQ_LOW_PRIORITY, irqHandlers[2] );
        DIO2_pin::SetInterrupt(InterruptMode::RisingEdge);
        DIO2_pin::GetIsrDelegate().template init<LoraModem, &LoraModem::SX1276OnDio2Irq>(this);
        // GpioSetInterrupt( &SX1276.DIO3, IRQ_RISING_EDGE, IRQ_LOW_PRIORITY, irqHandlers[3] );
        DIO3_pin::SetInterrupt(InterruptMode::RisingEdge);
        DIO3_pin::GetIsrDelegate().template init<LoraModem, &LoraModem::SX1276OnDio3Irq>(this);
        // GpioSetInterrupt( &SX1276.DIO4, IRQ_RISING_EDGE, IRQ_LOW_PRIORITY, irqHandlers[4] );
        DIO4_pin::SetInterrupt(InterruptMode::RisingEdge);
        DIO4_pin::GetIsrDelegate().template init<LoraModem, &LoraModem::SX1276OnDio4Irq>(this);
        // GpioSetInterrupt( &SX1276.DIO5, IRQ_RISING_EDGE, IRQ_LOW_PRIORITY, irqHandlers[5] );
    }

    void SX1276SetModem(RadioModems_t modem)
    {
        if ((SX1276Read(REG_OPMODE) & RFLR_OPMODE_LONGRANGEMODE_ON) != 0)
        {
            Settings.Modem = MODEM_LORA;
        }
        else
        {
            Settings.Modem = MODEM_FSK;
        }

        if (Settings.Modem == modem)
        {
            return;
        }

        Settings.Modem = modem;
        switch (Settings.Modem)
        {
        default:
        case MODEM_FSK:
            SX1276SetOpMode(RF_OPMODE_SLEEP);
            SX1276Write(REG_OPMODE,
                        (SX1276Read(REG_OPMODE) & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_OFF);

            SX1276Write(REG_DIOMAPPING1, 0x00);
            SX1276Write(REG_DIOMAPPING2, 0x30); // DIO5=ModeReady
            break;
        case MODEM_LORA:
            SX1276SetOpMode(RF_OPMODE_SLEEP);
            SX1276Write(REG_OPMODE,
                        (SX1276Read(REG_OPMODE) & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_ON);

            SX1276Write(REG_DIOMAPPING1, 0x00);
            SX1276Write(REG_DIOMAPPING2, 0x00);
            break;
        }
    }

    void SX1276Write(uint32_t addr, uint8_t data)
    {
        SX1276WriteBuffer(addr, &data, 1);
    }

    uint8_t SX1276Read(uint32_t addr)
    {
        uint8_t data;
        SX1276ReadBuffer(addr, &data, 1);
        return data;
    }

    void SX1276ReadBuffer(uint32_t addr, uint8_t *buffer, uint8_t size)
    {
        uint8_t i;

        // NSS = 0;
        NSS_pin::Reset();
        // GpioWrite( &SX1276.Spi.Nss, 0 );

        _spi->tx_byte(addr & 0x7F);
        // SpiInOut( &SX1276.Spi, addr & 0x7F );

        for (i = 0; i < size; i++)
        {
            buffer[i] = _spi->tx_byte(0);
            // buffer[i] = SpiInOut( &SX1276.Spi, 0 );
        }

        // NSS = 1;
        NSS_pin::Set();
        // GpioWrite( &SX1276.Spi.Nss, 1 );
    }

    uint32_t SX1276ConvertPllStepToFreqInHz(uint32_t pllSteps)
    {
        uint32_t freqInHzInt;
        uint32_t freqInHzFrac;

        // freqInHz = pllSteps * ( SX1276_XTAL_FREQ / 2^19 )
        // Get integer and fractional parts of the frequency computed with a PLL step scaled value
        freqInHzInt = pllSteps >> SX1276_PLL_STEP_SHIFT_AMOUNT;
        freqInHzFrac = pllSteps - (freqInHzInt << SX1276_PLL_STEP_SHIFT_AMOUNT);

        // Apply the scaling factor to retrieve a frequency in Hz (+ ceiling)
        return freqInHzInt * SX1276_PLL_STEP_SCALED +
               ((freqInHzFrac * SX1276_PLL_STEP_SCALED + (128)) >> SX1276_PLL_STEP_SHIFT_AMOUNT);
    }

    void SX1276WriteBuffer(uint32_t addr, uint8_t *buffer, uint8_t size)
    {
        uint8_t i;

        // NSS = 0;
        NSS_pin::Reset();
        // GpioWrite(&SX1276.Spi.Nss, 0);

        _spi->tx_byte(addr | 0x80);
        // SpiInOut(&SX1276.Spi, addr | 0x80);
        for (i = 0; i < size; i++)
        {
            _spi->tx_byte(buffer[i]);
            // SpiInOut(&SX1276.Spi, buffer[i]);
        }

        // NSS = 1;
        NSS_pin::Set();
        // GpioWrite(&SX1276.Spi.Nss, 1);
    }

    uint32_t SX1276ConvertFreqInHzToPllStep(uint32_t freqInHz)
    {
        uint32_t stepsInt;
        uint32_t stepsFrac;

        // pllSteps = freqInHz / (SX1276_XTAL_FREQ / 2^19 )
        // Get integer and fractional parts of the frequency computed with a PLL step scaled value
        stepsInt = freqInHz / SX1276_PLL_STEP_SCALED;
        stepsFrac = freqInHz - (stepsInt * SX1276_PLL_STEP_SCALED);

        // Apply the scaling factor to retrieve a frequency in Hz (+ ceiling)
        return (stepsInt << SX1276_PLL_STEP_SHIFT_AMOUNT) +
               (((stepsFrac << SX1276_PLL_STEP_SHIFT_AMOUNT) + (SX1276_PLL_STEP_SCALED >> 1)) / SX1276_PLL_STEP_SCALED);
    }

    uint8_t SX1276GetPaSelect(uint32_t channel)
    {
        return RF_PACONFIG_PASELECT_PABOOST; //мощность
    }

    void SX1276SetRfTxPower(int8_t power)
    {
        uint8_t paConfig = 0;
        uint8_t paDac = 0;

        paConfig = SX1276Read(REG_PACONFIG);
        paDac = SX1276Read(REG_PADAC);

        paConfig = (paConfig & RF_PACONFIG_PASELECT_MASK) | SX1276GetPaSelect(Settings.Channel);

        if ((paConfig & RF_PACONFIG_PASELECT_PABOOST) == RF_PACONFIG_PASELECT_PABOOST)
        {
            if (power > 17)
            {
                paDac = (paDac & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_ON;
            }
            else
            {
                paDac = (paDac & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_OFF;
            }
            if ((paDac & RF_PADAC_20DBM_ON) == RF_PADAC_20DBM_ON)
            {
                if (power < 5)
                {
                    power = 5;
                }
                if (power > 20)
                {
                    power = 20;
                }
                paConfig = (paConfig & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)((uint16_t)(power - 5) & 0x0F);
            }
            else
            {
                if (power < 2)
                {
                    power = 2;
                }
                if (power > 17)
                {
                    power = 17;
                }
                paConfig = (paConfig & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)((uint16_t)(power - 2) & 0x0F);
            }
        }
        else
        {
            if (power > 0)
            {
                if (power > 15)
                {
                    power = 15;
                }
                paConfig = (paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK) | (7 << 4) | (power);
            }
            else
            {
                if (power < -4)
                {
                    power = -4;
                }
                paConfig =
                    (paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK) | (0 << 4) | (power + 4);
            }
        }
        SX1276Write(REG_PACONFIG, paConfig);
        SX1276Write(REG_PADAC, paDac);
    }

    void SX1276WriteFifo(uint8_t *buffer, uint8_t size)
    {
        SX1276WriteBuffer(0, buffer, size);
    }

    void SX1276OnDio0Irq()
    {
        volatile uint8_t irqFlags = 0;

        switch (Settings.State)
        {
        case RF_RX_RUNNING:
            // TimerStop( &RxTimeoutTimer );
            //  RxDone interrupt
            switch (Settings.Modem)
            {
            case MODEM_FSK:
                if (Settings.Fsk.CrcOn == true)
                {
                    irqFlags = SX1276Read(REG_IRQFLAGS2);
                    if ((irqFlags & RF_IRQFLAGS2_CRCOK) != RF_IRQFLAGS2_CRCOK)
                    {
                        // Clear Irqs
                        SX1276Write(REG_IRQFLAGS1,
                                    RF_IRQFLAGS1_RSSI | RF_IRQFLAGS1_PREAMBLEDETECT | RF_IRQFLAGS1_SYNCADDRESSMATCH);
                        SX1276Write(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);

                        // TimerStop(&RxTimeoutTimer);
                        RxTimeoutTimer.stop();

                        if (Settings.Fsk.RxContinuous == false)
                        {
                            // TimerStop(&RxTimeoutSyncWord);
                            RxTimeoutSyncWord.stop();
                            Settings.State = RF_IDLE;
                        }
                        else
                        {
                            // Continuous mode restart Rx chain
                            SX1276Write(REG_RXCONFIG, SX1276Read(REG_RXCONFIG) | RF_RXCONFIG_RESTARTRXWITHOUTPLLLOCK);
                        }

                        // if ((RadioEvents != NULL) && (RadioEvents->RxError != NULL))
                        //{
                        // RadioEvents->RxError();
                        RxError(); // CRC non correct
                        //}
                        Settings.FskPacketHandler.PreambleDetected = false;
                        Settings.FskPacketHandler.SyncWordDetected = false;
                        Settings.FskPacketHandler.NbBytes = 0;
                        Settings.FskPacketHandler.Size = 0;
                        break;
                    }
                }
                // CRC OK
                //  Read received packet size
                if ((Settings.FskPacketHandler.Size == 0) && (Settings.FskPacketHandler.NbBytes == 0))
                {
                    if (Settings.Fsk.FixLen == false)
                    {
                        SX1276ReadFifo((uint8_t *)&Settings.FskPacketHandler.Size, 1);
                    }
                    else
                    {
                        Settings.FskPacketHandler.Size = SX1276Read(REG_PAYLOADLENGTH);
                    }
                    SX1276ReadFifo(RxTxBuffer + Settings.FskPacketHandler.NbBytes,
                                   Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes);
                    Settings.FskPacketHandler.NbBytes +=
                        (Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes);
                }
                else
                {
                    SX1276ReadFifo(RxTxBuffer + Settings.FskPacketHandler.NbBytes,
                                   Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes);
                    Settings.FskPacketHandler.NbBytes +=
                        (Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes);
                }

                // TimerStop(&RxTimeoutTimer);
                RxTimeoutTimer.stop();

                if (Settings.Fsk.RxContinuous == false)
                {
                    Settings.State = RF_IDLE;
                    // TimerStop(&RxTimeoutSyncWord);
                    RxTimeoutSyncWord.stop();
                }
                else
                {
                    // Continuous mode restart Rx chain
                    SX1276Write(REG_RXCONFIG, SX1276Read(REG_RXCONFIG) | RF_RXCONFIG_RESTARTRXWITHOUTPLLLOCK);
                }

                // if ((RadioEvents != NULL) && (RadioEvents->RxDone != NULL))
                //{
                // RadioEvents->RxDone(RxTxBuffer, SX1276.Settings.FskPacketHandler.Size,
                //                     SX1276.Settings.FskPacketHandler.RssiValue, 0);
                RxDone(RxTxBuffer, Settings.FskPacketHandler.Size, Settings.FskPacketHandler.RssiValue, 0);
                //}
                Settings.FskPacketHandler.PreambleDetected = false;
                Settings.FskPacketHandler.SyncWordDetected = false;
                Settings.FskPacketHandler.NbBytes = 0;
                Settings.FskPacketHandler.Size = 0;
                break;
            case MODEM_LORA: {
                // Clear Irq
                SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXDONE);

                irqFlags = SX1276Read(REG_LR_IRQFLAGS);
                if ((irqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR_MASK) == RFLR_IRQFLAGS_PAYLOADCRCERROR)
                {
                    // Clear Irq
                    SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_PAYLOADCRCERROR);

                    if (Settings.LoRa.RxContinuous == false)
                    {
                        Settings.State = RF_IDLE;
                    }
                    // TimerStop(&RxTimeoutTimer);
                    RxTimeoutTimer.stop();

                    // if ((RadioEvents != NULL) && (RadioEvents->RxError != NULL))
                    //{
                    //     RadioEvents->RxError();
                    RxError();
                    //}
                    break;
                }

                // Returns SNR value [dB] rounded to the nearest integer value
                Settings.LoRaPacketHandler.SnrValue = (((int8_t)SX1276Read(REG_LR_PKTSNRVALUE)) + 2) >> 2;

                int16_t rssi = SX1276Read(REG_LR_PKTRSSIVALUE);
                if (Settings.LoRaPacketHandler.SnrValue < 0)
                {
                    if (Settings.Channel > RF_MID_BAND_THRESH)
                    {
                        Settings.LoRaPacketHandler.RssiValue =
                            RSSI_OFFSET_HF + rssi + (rssi >> 4) + Settings.LoRaPacketHandler.SnrValue;
                    }
                    else
                    {
                        Settings.LoRaPacketHandler.RssiValue =
                            RSSI_OFFSET_LF + rssi + (rssi >> 4) + Settings.LoRaPacketHandler.SnrValue;
                    }
                }
                else
                {
                    if (Settings.Channel > RF_MID_BAND_THRESH)
                    {
                        Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_HF + rssi + (rssi >> 4);
                    }
                    else
                    {
                        Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_LF + rssi + (rssi >> 4);
                    }
                }

                Settings.LoRaPacketHandler.Size = SX1276Read(REG_LR_RXNBBYTES);
                SX1276Write(REG_LR_FIFOADDRPTR, SX1276Read(REG_LR_FIFORXCURRENTADDR));
                SX1276ReadFifo(RxTxBuffer, Settings.LoRaPacketHandler.Size);

                if (Settings.LoRa.RxContinuous == false)
                {
                    Settings.State = RF_IDLE;
                }
                // TimerStop(&RxTimeoutTimer);
                RxTimeoutTimer.stop();

                // if ((RadioEvents != NULL) && (RadioEvents->RxDone != NULL))
                //{
                //     RadioEvents->RxDone(RxTxBuffer, SX1276.Settings.LoRaPacketHandler.Size,
                //                         SX1276.Settings.LoRaPacketHandler.RssiValue,
                //                         SX1276.Settings.LoRaPacketHandler.SnrValue);
                RxDone(RxTxBuffer, Settings.LoRaPacketHandler.Size, Settings.LoRaPacketHandler.RssiValue,
                       Settings.LoRaPacketHandler.SnrValue);
                //}
            }
            break;
            default:
                break;
            }
            break;
        case RF_TX_RUNNING:
            // TimerStop(&TxTimeoutTimer);
            TxTimeoutTimer.stop();
            // TxDone interrupt
            switch (Settings.Modem)
            {
            case MODEM_LORA:
                // Clear Irq
                SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE);
                // Intentional fall through
            case MODEM_FSK:
            default:
                Settings.State = RF_IDLE;
                // if ((RadioEvents != NULL) && (RadioEvents->TxDone != NULL))
                //{
                //     RadioEvents->TxDone();
                TxDone();
                //}
                break;
            }
            break;
        default:
            break;
        }
    }

    void SX1276OnDio1Irq()
    {
        switch (Settings.State)
        {
        case RF_RX_RUNNING:
            switch (Settings.Modem)
            {
            case MODEM_FSK:
                // Check FIFO level DIO1 pin state
                //
                // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
                // verify DIO1 pin state in order to decide if something has to be done.
                // When radio is operating in FSK reception mode a rising edge must be detected in order to handle
                // the IRQ. if (SX1276GetDio1PinState() == 0)
                if (DIO1_pin::Read() == false)
                {
                    break;
                }
                // Stop timer
                // TimerStop(&RxTimeoutSyncWord);
                RxTimeoutSyncWord.stop();
                // FifoLevel interrupt
                // Read received packet size
                if ((Settings.FskPacketHandler.Size == 0) && (Settings.FskPacketHandler.NbBytes == 0))
                {
                    if (Settings.Fsk.FixLen == false)
                    {
                        SX1276ReadFifo((uint8_t *)&Settings.FskPacketHandler.Size, 1);
                    }
                    else
                    {
                        Settings.FskPacketHandler.Size = SX1276Read(REG_PAYLOADLENGTH);
                    }
                }

                // ERRATA 3.1 - PayloadReady Set for 31.25ns if FIFO is Empty
                //
                //              When FifoLevel interrupt is used to offload the
                //              FIFO, the microcontroller should  monitor  both
                //              PayloadReady  and FifoLevel interrupts, and
                //              read only (FifoThreshold-1) bytes off the FIFO
                //              when FifoLevel fires
                if ((Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes) >=
                    Settings.FskPacketHandler.FifoThresh)
                {
                    SX1276ReadFifo((RxTxBuffer + Settings.FskPacketHandler.NbBytes),
                                   Settings.FskPacketHandler.FifoThresh - 1);
                    Settings.FskPacketHandler.NbBytes += Settings.FskPacketHandler.FifoThresh - 1;
                }
                else
                {
                    SX1276ReadFifo((RxTxBuffer + Settings.FskPacketHandler.NbBytes),
                                   Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes);
                    Settings.FskPacketHandler.NbBytes +=
                        (Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes);
                }
                break;
            case MODEM_LORA:
                // Check RxTimeout DIO1 pin state
                //
                // DIO1 irq is setup to be triggered on rsing and falling edges
                // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
                // verify DIO1 pin state in order to decide if something has to be done.
                // When radio is operating in LoRa reception mode a rising edge must be detected in order to handle
                // the IRQ. if (SX1276GetDio1PinState() == 0)
                if (DIO1_pin::Read() == false)
                {
                    break;
                }
                // Sync time out
                // TimerStop(&RxTimeoutTimer);
                RxTimeoutTimer.stop();
                // Clear Irq
                SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXTIMEOUT);

                Settings.State = RF_IDLE;
                //                if ((RadioEvents != NULL) && (RadioEvents->RxTimeout != NULL))
                //                {
                //                    RadioEvents->RxTimeout();
                RxTimeout();
                //                }
                break;
            default:
                break;
            }
            break;
        case RF_TX_RUNNING:
            switch (Settings.Modem)
            {
            case MODEM_FSK:
                // Check FIFO level DIO1 pin state
                //
                // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
                // verify DIO1 pin state in order to decide if something has to be done.
                // When radio is operating in FSK transmission mode a falling edge must be detected in order to
                // handle the IRQ. if (SX1276GetDio1PinState() == 1)
                if (DIO1_pin::Read() == true)
                {
                    break;
                }

                // FifoLevel interrupt
                if ((Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes) >
                    Settings.FskPacketHandler.ChunkSize)
                {
                    SX1276WriteFifo((RxTxBuffer + Settings.FskPacketHandler.NbBytes),
                                    Settings.FskPacketHandler.ChunkSize);
                    Settings.FskPacketHandler.NbBytes += Settings.FskPacketHandler.ChunkSize;
                }
                else
                {
                    // Write the last chunk of data
                    SX1276WriteFifo(RxTxBuffer + Settings.FskPacketHandler.NbBytes,
                                    Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes);
                    Settings.FskPacketHandler.NbBytes +=
                        Settings.FskPacketHandler.Size - Settings.FskPacketHandler.NbBytes;
                }
                break;
            case MODEM_LORA:
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    void SX1276OnDio2Irq()
    {
        switch (Settings.State)
        {
        case RF_RX_RUNNING:
            switch (Settings.Modem)
            {
            case MODEM_FSK:
                // Checks if DIO4 is connected. If it is not PreambleDetected is set to true.
                // if (SX1276.DIO4.port == NULL)
                if (DIO4_pin::exist() == false)
                {
                    Settings.FskPacketHandler.PreambleDetected = true;
                }

                if ((Settings.FskPacketHandler.PreambleDetected != 0) &&
                    (Settings.FskPacketHandler.SyncWordDetected == 0))
                {
                    // TimerStop(&RxTimeoutSyncWord);
                    RxTimeoutSyncWord.stop();
                    Settings.FskPacketHandler.SyncWordDetected = true;

                    Settings.FskPacketHandler.RssiValue = -(SX1276Read(REG_RSSIVALUE) >> 1);

                    Settings.FskPacketHandler.AfcValue = (int32_t)SX1276ConvertPllStepToFreqInHz(
                        ((uint16_t)SX1276Read(REG_AFCMSB) << 8) | (uint16_t)SX1276Read(REG_AFCLSB));
                    Settings.FskPacketHandler.RxGain = (SX1276Read(REG_LNA) >> 5) & 0x07;
                }
                break;
            case MODEM_LORA:
                if (Settings.LoRa.FreqHopOn == true)
                {
                    // Clear Irq
                    SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL);

                    // if ((RadioEvents != NULL) && (RadioEvents->FhssChangeChannel != NULL))
                    //{
                    //     RadioEvents->FhssChangeChannel((SX1276Read(REG_LR_HOPCHANNEL) &
                    //     RFLR_HOPCHANNEL_CHANNEL_MASK));
                    FhssChangeChannel((SX1276Read(REG_LR_HOPCHANNEL) & RFLR_HOPCHANNEL_CHANNEL_MASK));
                    //}
                }
                break;
            default:
                break;
            }
            break;
        case RF_TX_RUNNING:
            switch (Settings.Modem)
            {
            case MODEM_FSK:
                break;
            case MODEM_LORA:
                if (Settings.LoRa.FreqHopOn == true)
                {
                    // Clear Irq
                    SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL);

                    // if ((RadioEvents != NULL) && (RadioEvents->FhssChangeChannel != NULL))
                    //{
                    //     RadioEvents->FhssChangeChannel((SX1276Read(REG_LR_HOPCHANNEL) &
                    //     RFLR_HOPCHANNEL_CHANNEL_MASK));
                    FhssChangeChannel((SX1276Read(REG_LR_HOPCHANNEL) & RFLR_HOPCHANNEL_CHANNEL_MASK));
                    //}
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    void SX1276OnDio3Irq()
    {
        switch (Settings.Modem)
        {
        case MODEM_FSK:
            break;
        case MODEM_LORA:
            if ((SX1276Read(REG_LR_IRQFLAGS) & RFLR_IRQFLAGS_CADDETECTED) == RFLR_IRQFLAGS_CADDETECTED)
            {
                // Clear Irq
                SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDETECTED | RFLR_IRQFLAGS_CADDONE);
                // if ((RadioEvents != NULL) && (RadioEvents->CadDone != NULL))
                //{
                //     RadioEvents->CadDone(true);
                CadDone(true);
                //}
            }
            else
            {
                // Clear Irq
                SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDONE);
                // if ((RadioEvents != NULL) && (RadioEvents->CadDone != NULL))
                //{
                //     RadioEvents->CadDone(false);
                CadDone(false);
                //}
            }
            break;
        default:
            break;
        }
    }

    void SX1276OnDio4Irq()
    {
        switch (Settings.Modem)
        {
        case MODEM_FSK: {
            if (Settings.FskPacketHandler.PreambleDetected == false)
            {
                Settings.FskPacketHandler.PreambleDetected = true;
            }
        }
        break;
        case MODEM_LORA:
            break;
        default:
            break;
        }
    }

    void SX1276ReadFifo(uint8_t *buffer, uint8_t size)
    {
        SX1276ReadBuffer(0, buffer, size);
    }

    uint8_t GetFskBandwidthRegValue(uint32_t bw)
    {
        uint8_t i;

        for (i = 0; i < FskBandwidths.size(); i++)
        {
            if ((bw >= FskBandwidths[i].bandwidth) && (bw < FskBandwidths[i + 1].bandwidth))
            {
                return FskBandwidths[i].RegValue;
            }
        }
        // ERROR: Value not found
        while (1)
            ;
    }

  private:
    SPI_MasterBase *_spi;
    TIMER TxTimeoutTimer;
    TIMER RxTimeoutTimer;
    TIMER RxTimeoutSyncWord;
    RadioSettings_t Settings;
    uint8_t RxTxBuffer[RX_TX_BUFFER_SIZE];

  public:
    Delegate<void()> RxTimeoutEvent;
    Delegate<void()> RxError;
    Delegate<void()> TxDone;
    /*!
     * Rx Done callback prototype.
     *
     * [IN] payload Received buffer pointer
     * [IN] size    Received buffer size
     * [IN] rssi    RSSI value computed while receiving the frame [dBm]
     * [IN] snr     SNR value computed while receiving the frame [dB]
     *                     FSK : N/A ( set to 0 )
     *                     LoRa: SNR value in dB
     void RxDoneCallback(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);*/
    Delegate<void(uint8_t *, uint16_t, int16_t, int8_t)> RxDone;
    Delegate<void()> RxTimeout;
    Delegate<void(uint8_t)> FhssChangeChannel;
    Delegate<void(bool)> CadDone;
};
//
///*!
// * Radio wake-up time from sleep
// */
//#define RADIO_WAKEUP_TIME                           1 // [ms]
//
///*!
// * Sync word for Private LoRa networks
// */
//#define LORA_MAC_PRIVATE_SYNCWORD                   0x12
//
///*!
// * Sync word for Public LoRa networks
// */
//#define LORA_MAC_PUBLIC_SYNCWORD                    0x34
//
///*!
// * Radio FSK modem parameters
// */
// typedef struct
//{
//    int8_t   Power;
//    uint32_t Fdev;
//    uint32_t Bandwidth;
//    uint32_t BandwidthAfc;
//    uint32_t Datarate;
//    uint16_t PreambleLen;
//    bool     FixLen;
//    uint8_t  PayloadLen;
//    bool     CrcOn;
//    bool     IqInverted;
//    bool     RxContinuous;
//    uint32_t TxTimeout;
//    uint32_t RxSingleTimeout;
//}RadioFskSettings_t;
//
///*!
// * Radio FSK packet handler state
// */
// typedef struct
//{
//    uint8_t  PreambleDetected;
//    uint8_t  SyncWordDetected;
//    int8_t   RssiValue;
//    int32_t  AfcValue;
//    uint8_t  RxGain;
//    uint16_t Size;
//    uint16_t NbBytes;
//    uint8_t  FifoThresh;
//    uint8_t  ChunkSize;
//}RadioFskPacketHandler_t;
//
///*!
// * Radio LoRa modem parameters
// */
// typedef struct
//{
//    int8_t   Power;
//    uint32_t Bandwidth;
//    uint32_t Datarate;
//    bool     LowDatarateOptimize;
//    uint8_t  Coderate;
//    uint16_t PreambleLen;
//    bool     FixLen;
//    uint8_t  PayloadLen;
//    bool     CrcOn;
//    bool     FreqHopOn;
//    uint8_t  HopPeriod;
//    bool     IqInverted;
//    bool     RxContinuous;
//    uint32_t TxTimeout;
//    bool     PublicNetwork;
//}RadioLoRaSettings_t;
//
///*!
// * Radio LoRa packet handler state
// */
// typedef struct
//{
//    int8_t SnrValue;
//    int16_t RssiValue;
//    uint8_t Size;
//}RadioLoRaPacketHandler_t;
//
///*!
// * Radio Settings
// */
// typedef struct
//{
//    RadioState_t             State;
//    RadioModems_t            Modem;
//    uint32_t                 Channel;
//    RadioFskSettings_t       Fsk;
//    RadioFskPacketHandler_t  FskPacketHandler;
//    RadioLoRaSettings_t      LoRa;
//    RadioLoRaPacketHandler_t LoRaPacketHandler;
//}RadioSettings_t;
//
///*!
// * Radio hardware and global parameters
// */
// typedef struct SX1276_s
//{
//    Gpio_t        Reset;
//    Gpio_t        DIO0;
//    Gpio_t        DIO1;
//    Gpio_t        DIO2;
//    Gpio_t        DIO3;
//    Gpio_t        DIO4;
//    Gpio_t        DIO5;
//    Spi_t         Spi;
//    RadioSettings_t Settings;
//}SX1276_t;
//
///*!
// * Hardware IO IRQ callback function definition
// */
// typedef void ( DioIrqHandler )( void* context );
//
///*
// * SX1276 definitions
// */
//
///*!
// * ============================================================================
// * Public functions prototypes
// * ============================================================================
// */
//
///*!
// * \brief Initializes the radio
// *
// * \param [IN] events Structure containing the driver callback functions
// */
// void SX1276Init( RadioEvents_t *events );
//
///*!
// * Return current radio status
// *
// * \param status Radio status.[RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
// */
// RadioState_t SX1276GetStatus( void );
//
///*!
// * \brief Configures the radio with the given modem
// *
// * \param [IN] modem Modem to be used [0: FSK, 1: LoRa]
// */
// void SX1276SetModem( RadioModems_t modem );
//
///*!
// * \brief Sets the channel configuration
// *
// * \param [IN] freq         Channel RF frequency
// */
// void SX1276SetChannel( uint32_t freq );
//
///*!
// * \brief Checks if the channel is free for the given time
// *
// * \remark The FSK modem is always used for this task as we can select the Rx bandwidth at will.
// *
// * \param [IN] freq                Channel RF frequency in Hertz
// * \param [IN] rxBandwidth         Rx bandwidth in Hertz
// * \param [IN] rssiThresh          RSSI threshold in dBm
// * \param [IN] maxCarrierSenseTime Max time in milliseconds while the RSSI is measured
// *
// * \retval isFree         [true: Channel is free, false: Channel is not free]
// */
// bool SX1276IsChannelFree( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime
// );
//
///*!
// * \brief Generates a 32 bits random value based on the RSSI readings
// *
// * \remark This function sets the radio in LoRa modem mode and disables
// *         all interrupts.
// *         After calling this function either SX1276SetRxConfig or
// *         SX1276SetTxConfig functions must be called.
// *
// * \retval randomValue    32 bits random value
// */
// uint32_t SX1276Random( void );
//
///*!
// * \brief Sets the reception parameters
// *
// * \remark When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
// *
// * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
// * \param [IN] bandwidth    Sets the bandwidth
// *                          FSK : >= 2600 and <= 250000 Hz
// *                          LoRa: [0: 125 kHz, 1: 250 kHz,
// *                                 2: 500 kHz, 3: Reserved]
// * \param [IN] datarate     Sets the Datarate
// *                          FSK : 600..300000 bits/s
// *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
// *                                10: 1024, 11: 2048, 12: 4096  chips]
// * \param [IN] coderate     Sets the coding rate (LoRa only)
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
// * \param [IN] bandwidthAfc Sets the AFC Bandwidth (FSK only)
// *                          FSK : >= 2600 and <= 250000 Hz
// *                          LoRa: N/A ( set to 0 )
// * \param [IN] preambleLen  Sets the Preamble length
// *                          FSK : Number of bytes
// *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
// * \param [IN] symbTimeout  Sets the RxSingle timeout value
// *                          FSK : timeout number of bytes
// *                          LoRa: timeout in symbols
// * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
// * \param [IN] payloadLen   Sets payload length when fixed length is used
// * \param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
// * \param [IN] freqHopOn    Enables disables the intra-packet frequency hopping
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [0: OFF, 1: ON]
// * \param [IN] hopPeriod    Number of symbols between each hop
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: Number of symbols
// * \param [IN] iqInverted   Inverts IQ signals (LoRa only)
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [0: not inverted, 1: inverted]
// * \param [IN] rxContinuous Sets the reception in continuous mode
// *                          [false: single mode, true: continuous mode]
// */
// void SX1276SetRxConfig( RadioModems_t modem, uint32_t bandwidth,
//                         uint32_t datarate, uint8_t coderate,
//                         uint32_t bandwidthAfc, uint16_t preambleLen,
//                         uint16_t symbTimeout, bool fixLen,
//                         uint8_t payloadLen,
//                         bool crcOn, bool freqHopOn, uint8_t hopPeriod,
//                         bool iqInverted, bool rxContinuous );
//
///*!
// * \brief Sets the transmission parameters
// *
// * \remark When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
// *
// * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
// * \param [IN] power        Sets the output power [dBm]
// * \param [IN] fdev         Sets the frequency deviation (FSK only)
// *                          FSK : [Hz]
// *                          LoRa: 0
// * \param [IN] bandwidth    Sets the bandwidth (LoRa only)
// *                          FSK : 0
// *                          LoRa: [0: 7.8 kHz, 1: 10.4 kHz, 2: 15.6 kHz
// *                                 2: 500 kHz, 3: Reserved]
// * \param [IN] datarate     Sets the Datarate
// *                          FSK : 600..300000 bits/s
// *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
// *                                10: 1024, 11: 2048, 12: 4096  chips]
// * \param [IN] coderate     Sets the coding rate (LoRa only)
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
// * \param [IN] preambleLen  Sets the preamble length
// *                          FSK : Number of bytes
// *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
// * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
// * \param [IN] crcOn        Enables disables the CRC [0: OFF, 1: ON]
// * \param [IN] freqHopOn    Enables disables the intra-packet frequency hopping
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [0: OFF, 1: ON]
// * \param [IN] hopPeriod    Number of symbols between each hop
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: Number of symbols
// * \param [IN] iqInverted   Inverts IQ signals (LoRa only)
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [0: not inverted, 1: inverted]
// * \param [IN] timeout      Transmission timeout [ms]
// */
// void SX1276SetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
//                        uint32_t bandwidth, uint32_t datarate,
//                        uint8_t coderate, uint16_t preambleLen,
//                        bool fixLen, bool crcOn, bool freqHopOn,
//                        uint8_t hopPeriod, bool iqInverted, uint32_t timeout );
//
///*!
// * \brief Computes the packet time on air in ms for the given payload
// *
// * \Remark Can only be called once SetRxConfig or SetTxConfig have been called
// *
// * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
// * \param [IN] bandwidth    Sets the bandwidth
// *                          FSK : >= 2600 and <= 250000 Hz
// *                          LoRa: [0: 125 kHz, 1: 250 kHz,
// *                                 2: 500 kHz, 3: Reserved]
// * \param [IN] datarate     Sets the Datarate
// *                          FSK : 600..300000 bits/s
// *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
// *                                10: 1024, 11: 2048, 12: 4096  chips]
// * \param [IN] coderate     Sets the coding rate (LoRa only)
// *                          FSK : N/A ( set to 0 )
// *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
// * \param [IN] preambleLen  Sets the Preamble length
// *                          FSK : Number of bytes
// *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
// * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
// * \param [IN] payloadLen   Sets payload length when fixed length is used
// * \param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
// *
// * \retval airTime        Computed airTime (ms) for the given packet payload length
// */
// uint32_t SX1276GetTimeOnAir( RadioModems_t modem, uint32_t bandwidth,
//                              uint32_t datarate, uint8_t coderate,
//                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
//                              bool crcOn );
//
///*!
// * \brief Sends the buffer of size. Prepares the packet to be sent and sets
// *        the radio in transmission
// *
// * \param [IN]: buffer     Buffer pointer
// * \param [IN]: size       Buffer size
// */
// void SX1276Send( uint8_t *buffer, uint8_t size );
//
///*!
// * \brief Sets the radio in sleep mode
// */
// void SX1276SetSleep( void );
//
///*!
// * \brief Sets the radio in standby mode
// */
// void SX1276SetStby( void );
//
///*!
// * \brief Sets the radio in reception mode for the given time
// * \param [IN] timeout Reception timeout [ms] [0: continuous, others timeout]
// */
// void SX1276SetRx( uint32_t timeout );
//
///*!
// * \brief Start a Channel Activity Detection
// */
// void SX1276StartCad( void );
//
///*!
// * \brief Sets the radio in continuous wave transmission mode
// *
// * \param [IN]: freq       Channel RF frequency
// * \param [IN]: power      Sets the output power [dBm]
// * \param [IN]: time       Transmission mode timeout [s]
// */
// void SX1276SetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time );
//
///*!
// * \brief Reads the current RSSI value
// *
// * \retval rssiValue Current RSSI value in [dBm]
// */
// int16_t SX1276ReadRssi( RadioModems_t modem );
//
///*!
// * \brief Writes the radio register at the specified address
// *
// * \param [IN]: addr Register address
// * \param [IN]: data New register value
// */
// void SX1276Write( uint32_t addr, uint8_t data );
//
///*!
// * \brief Reads the radio register at the specified address
// *
// * \param [IN]: addr Register address
// * \retval data Register value
// */
// uint8_t SX1276Read( uint32_t addr );
//
///*!
// * \brief Writes multiple radio registers starting at address
// *
// * \param [IN] addr   First Radio register address
// * \param [IN] buffer Buffer containing the new register's values
// * \param [IN] size   Number of registers to be written
// */
// void SX1276WriteBuffer( uint32_t addr, uint8_t *buffer, uint8_t size );
//
///*!
// * \brief Reads multiple radio registers starting at address
// *
// * \param [IN] addr First Radio register address
// * \param [OUT] buffer Buffer where to copy the registers data
// * \param [IN] size Number of registers to be read
// */
// void SX1276ReadBuffer( uint32_t addr, uint8_t *buffer, uint8_t size );
//
///*!
// * \brief Sets the maximum payload length.
// *
// * \param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
// * \param [IN] max        Maximum payload length in bytes
// */
// void SX1276SetMaxPayloadLength( RadioModems_t modem, uint8_t max );
//
///*!
// * \brief Sets the network to public or private. Updates the sync byte.
// *
// * \remark Applies to LoRa modem only
// *
// * \param [IN] enable if true, it enables a public network
// */
// void SX1276SetPublicNetwork( bool enable );
//
///*!
// * \brief Gets the time required for the board plus radio to get out of sleep.[ms]
// *
// * \retval time Radio plus board wakeup time in ms.
// */
// uint32_t SX1276GetWakeupTime( void );
//
//#ifdef __cplusplus
//}
//#endif
//
//#endif // __SX1276_H__
