#include "spi-board.h"
#include "SPI_PIO.h"
/*
SPI_MasterBase* lora_spi_object{nullptr};

void LoraSpiSystemInit(SPI_MasterBase & spi)
{
	lora_spi_object = &spi;
}
*/
void SpiInit(Spi_t* obj, SpiId_t spiId, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss)
{
	GpioInit(&(obj->Nss), nss, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1);
}

void SpiDeInit(Spi_t* obj)
{
}

void SpiFormat(Spi_t* obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave)
{
	
}

void SpiFrequency(Spi_t* obj, uint32_t hz){}

uint16_t SpiInOut(Spi_t* obj, uint16_t outData)
{
	auto * const spi = static_cast<SPI_MasterBase*>(obj->Sclk.port);
	if (spi)
	{
		return spi->tx_byte(outData);
	}
	return 0;
}
