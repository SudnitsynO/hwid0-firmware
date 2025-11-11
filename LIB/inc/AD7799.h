#pragma once
#include <stdint.h>
#include <cmsis_os.h>
#include "SPI_base.h"
#ifdef STM32F103xB
#include "stm32f1xx_hal.h"
#endif

class AD7799
{
public:
	enum class Channel : uint8_t
	{
		AIN1 = 0,
		AIN2 = 1,
		AIN3 = 2,
		AIN1_SHORT = 3,
		AVDD_MON = 7
	};

	enum class Gain : uint8_t
	{
		GAIN_1 = 0,
		GAIN_2 = 1,
		GAIN_4 = 2,
		GAIN_8 = 3,
		GAIN_16 = 4,
		GAIN_32 = 5,
		GAIN_64 = 6,
		GAIN_128 = 7
	};

	enum class DataRate : uint8_t
	{
		RATE_470_HZ = 1,
		RATE_242_HZ = 2,
		RATE_123_HZ = 3,
		RATE_62_HZ = 4,
		RATE_50_HZ = 5,
		RATE_39_HZ = 6,
		RATE_33_2_HZ = 7,
		RATE_19_6_HZ_90DB = 8,	//60hz
		RATE_16_7_HZ_80DB = 9,	//50hz
		RATE_16_7_HZ_65DB = 10,
		RATE_12_5_HZ_66DB = 11,
		RATE_10_HZ_69DB = 12,
		RATE_8_33_HZ_70DB = 13,
		RATE_6_25_HZ_72DB = 14,
		RATE_4_17_HZ_74DB = 15
	};

	enum class Mode : uint8_t
	{
		CONTINUOUS_CONVERSION = 0,
		SINGLE_CONVERSION = 1,
		IDLE = 2,
		POWER_DOWN = 3,
		INTERNAL_ZERO_SCALE_CALIBRATION = 4,
		INTERNAL_FULL_SCALE_CALIBRATION = 5,
		SYSTEM_ZERO_SCALE_CALIBRATION = 6,
		SYSTEM_FULL_SCALE_CALIBRATION = 7
	};


	void init(SPI_MasterBase * spi_bus,bool bipolar_mode, bool buf_enable);
	void set_mode(Mode mode);
	void set_channel(Channel channel);
	void set_gain(Gain gain);
	void set_data_rate(DataRate rate);
	void wait(TickType_t timeout);
	int32_t read_result();
	void set_psw(bool psw_enabled);
	void set_burnout_curent(bool burnout_curent_enabled);
	void reset();
private:
	void update_reg();
	SPI_MasterBase* _spi_bus = nullptr;
	uint16_t _mode_reg;
	uint16_t _config_reg;
	bool _bpmode;
};