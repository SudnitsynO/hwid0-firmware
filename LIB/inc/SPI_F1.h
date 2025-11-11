#pragma once
#include "SPI_base.h"
#include <cmsis_os.h>
#ifdef STM32F103xB
#include "stm32f1xx_hal.h"
#endif

class SPI_F1 : public SPI_MasterBase
{
public:
	uint8_t tx_byte(const uint8_t byte) override;
	void tx_rx_buf(const uint8_t* const tx_buffer, uint8_t* const rx_buffer, const size_t data_count) override;
	void tx_buf(const uint8_t* const tx_buffer, const size_t data_count) override;
	void rx_buf(uint8_t* rx_buffer, const size_t data_count) override;
	void init(SPI_HandleTypeDef& spi_handle);
	TickType_t timeout = 1000;
	//interrupts
	void rx_cplt_isr();
	void cplt_isr();
	void flush() override;
private:
	SPI_HandleTypeDef * _spi = nullptr;
	//volatile bool _busy = false;
	volatile bool _init_complete = false;
	SemaphoreHandle_t _completed_semaphore = nullptr;
};

