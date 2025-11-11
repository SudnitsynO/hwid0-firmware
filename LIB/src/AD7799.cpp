#include "AD7799.h"
#include "BinarySerialize.h"
#include <delay.h>


void AD7799::init(SPI_MasterBase* spi_bus, const bool bipolar_mode, const bool buf_enable)
{
	_spi_bus = spi_bus;
	_mode_reg = 0x000A;
	_config_reg = 0x710;
	_bpmode = bipolar_mode;
	if (bipolar_mode)
		_config_reg &= ~(1 << 12);
	else
		_config_reg |= (1 << 12);
	if (!buf_enable)
		_config_reg &= ~(1 << 4);
	else
		_config_reg |= (1 << 4);
	reset();
	Delay_ms(500);
	update_reg();
}

void AD7799::set_mode(Mode mode)
{
	_mode_reg &= ~(7<<13);
	_mode_reg |= static_cast<uint16_t>(mode) << 13;
	update_reg();
}

void AD7799::set_channel(Channel channel)
{
	_config_reg &= ~7;
	_config_reg |= static_cast<uint16_t>(channel);
	update_reg();
}

void AD7799::set_gain(Gain gain)
{
	_config_reg &= ~(7<<8);
	_config_reg |= static_cast<uint16_t>(gain) << 8;
	update_reg();
}

void AD7799::set_data_rate(DataRate rate)
{
	_mode_reg &= ~(0x000F << 0);
	_mode_reg |= static_cast<uint16_t>(rate) << 0;
	update_reg();
}

void AD7799::wait(TickType_t timeout)
{
	auto start_time = xTaskGetTickCount();
	while ((xTaskGetTickCount() - start_time) < timeout)
	{
		_spi_bus->tx_byte((1 << 6) | 0);	//read status reg
		auto status = _spi_bus->tx_byte(0xFF);	//read status reg
		if((status & (1 << 7)) == 0)
			break;
		vTaskDelay(1);
	}
}

int32_t AD7799::read_result()
{
	_spi_bus->tx_byte((1<<6) | (3 << 3));	//read data reg
	uint8_t rx_data[3];
	rx_data[0] = 0xFF;
	rx_data[1] = 0xFF;
	rx_data[2] = 0xFF;
	_spi_bus->rx_buf(rx_data,3);
	int32_t result = static_cast<int32_t>(rx_data[2]) << 0 | static_cast<int32_t>(rx_data[1]) << 8 | static_cast<int32_t>(rx_data[0]) << 16;
	if (_bpmode)
		result -= 0x800000;
	return result;
}

void AD7799::set_psw(bool psw_enabled)
{
}

void AD7799::set_burnout_curent(bool burnout_curent_enabled)
{
}

void AD7799::reset()
{
	const std::vector<uint8_t> reset_sequence(5,0xFF);
	_spi_bus->tx_vector(reset_sequence);
	_spi_bus->flush();
}

void AD7799::update_reg()
{
	BinarySerializer bs;
	bs.serialise(uint8_t(1<<3));	//write to mode reg
	bs.serialise(uint8_t((_mode_reg & 0xFF00) >> 8));
	bs.serialise(uint8_t((_mode_reg & 0x00FF) >> 0));
	bs.serialise(uint8_t(2 << 3));	//write to config reg
	bs.serialise(uint8_t((_config_reg & 0xFF00) >> 8));
	bs.serialise(uint8_t((_config_reg & 0x00FF) >> 0));
	_spi_bus->tx_vector(bs.binary_data);
	_spi_bus->flush();
}
