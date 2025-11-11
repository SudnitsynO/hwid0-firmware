#include "SPI_F1.h"

uint8_t SPI_F1::tx_byte(const uint8_t byte)
{
	xSemaphoreTake(_completed_semaphore, timeout);
	uint8_t rx_byte = 0;
	uint8_t tx_byte = byte;
	HAL_SPI_TransmitReceive_IT(_spi, &tx_byte, &rx_byte, 1);
	flush();
	return rx_byte;
}

void SPI_F1::tx_rx_buf(const uint8_t* const tx_buffer, uint8_t* const rx_buffer, const size_t data_count)
{
	xSemaphoreTake(_completed_semaphore, timeout);
	HAL_SPI_TransmitReceive_DMA (_spi, const_cast<uint8_t*>(tx_buffer),rx_buffer, data_count);
	flush();
}

void SPI_F1::tx_buf(const uint8_t* const tx_buffer, const size_t data_count)
{
	xSemaphoreTake(_completed_semaphore,timeout);
	HAL_SPI_Transmit_DMA(_spi,const_cast<uint8_t*>(tx_buffer), data_count);
}

void SPI_F1::rx_buf(uint8_t* rx_buffer, const size_t data_count)
{
	xSemaphoreTake(_completed_semaphore, timeout);
	HAL_SPI_Receive_DMA(_spi, rx_buffer, data_count);
	flush();
}

void SPI_F1::flush()
{
	xSemaphoreTake(_completed_semaphore, timeout);
	xSemaphoreGive(_completed_semaphore);
}

void SPI_F1::init(SPI_HandleTypeDef& spi_handle)
{
	_spi = &spi_handle;
	_completed_semaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(_completed_semaphore);
	_init_complete = true;
}

void SPI_F1::cplt_isr()
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (!_init_complete) return;
	xSemaphoreGiveFromISR(_completed_semaphore, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken != pdFALSE)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

