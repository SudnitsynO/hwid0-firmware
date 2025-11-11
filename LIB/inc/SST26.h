#pragma once
#include "FlashDrive.h"
#include "SPI_base.h"
#include "gpio_interface.h"

template< class  CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
class Sst26Memory : public FlashDrive
{
public:
	void init(SPI_MasterBase * spi_bus);
	void test();
	uint32_t SectorSize() const override;
	uint32_t SectorInPageCount() const override;
	uint64_t PageCount() const override;
	bool ReadSector(uint32_t page_index, uint32_t sector_index, uint8_t* buffer) override;
	bool WriteSector(uint32_t page_index, uint32_t sector_index, uint8_t* buffer) override;
	bool ErasePage(uint32_t page_index) override;
	void wait();
private:
	SPI_MasterBase* _spi_bus = nullptr;
};

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
void Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::init(SPI_MasterBase* spi_bus)
{
	_spi_bus = spi_bus;
	CS_Line::Reset();
	_spi_bus->tx_byte(0x01);	//write config register 
	_spi_bus->tx_byte(0x00);
	_spi_bus->tx_byte(0x0A);	//disable WP# pin
	CS_Line::Set();

	CS_Line::Reset();
	_spi_bus->tx_byte(0x6);		//WREN
	CS_Line::Set();

	CS_Line::Reset();
	_spi_bus->tx_byte(0x98);	//ULBPR
	CS_Line::Set();
}

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
void Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::test()
{
	/*vector<uint8_t> buf(SectorSize(), 0xCC);
	WriteSector(0, 0, buf.data());*/
	
	ErasePage(0);

	vector<uint8_t> buf(SectorSize());
	ReadSector(0, 0, buf.data());
	for (size_t i = 0; i < 256; i++)
	{
		buf[i] = i;
	}
	WriteSector(0, 0, buf.data());
	ReadSector(0, 0, buf.data());
	for (size_t i = 0; i < 256; i++)
	{
		buf[i] = 0xFF;
	}
	buf[3] = 0;
	buf[5] = 0;
	WriteSector(0, 0, buf.data());
	ReadSector(0, 0, buf.data());

}

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
uint32_t Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::SectorSize() const
{
	return 256;
}

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
uint32_t Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::SectorInPageCount() const
{
	return  SECTOR_IN_PAGE_COUNT;
}

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
uint64_t Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::PageCount() const
{
	return PAGE_COUNT;
}

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
bool Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::ReadSector(uint32_t page_index, uint32_t sector_index,
	uint8_t* buffer)
{
	if (page_index >= PAGE_COUNT) return false;
	if (sector_index >= SECTOR_IN_PAGE_COUNT) return false;

	if (_spi_bus)
	{
		wait();
		CS_Line::Reset();
		_spi_bus->tx_byte(0x3);	//Read
		const uint16_t address = (page_index * SECTOR_IN_PAGE_COUNT) + sector_index;
		_spi_bus->tx_byte(address >> 8);	//ADDR2
		_spi_bus->tx_byte(address & 0xFF);	//ADDR1
		_spi_bus->tx_byte(0);				//ADDR0
		_spi_bus->rx_buf(buffer, 256);
		CS_Line::Set();
	}
	else
		return false;
	return true;
}

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
bool Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::WriteSector(uint32_t page_index, uint32_t sector_index,
	uint8_t* buffer)
{
	if (page_index >= PAGE_COUNT) return false;
	if(sector_index >= SECTOR_IN_PAGE_COUNT) return false;

	if(_spi_bus)
	{
		wait();
		CS_Line::Reset();
		_spi_bus->tx_byte(0x6);	//WREN
		CS_Line::Set();
		CS_Line::Reset();
		const uint16_t address = (page_index * SECTOR_IN_PAGE_COUNT) + sector_index;
		_spi_bus->tx_byte(0x2);	//WREN
		_spi_bus->tx_byte(address >> 8);	//ADDR2
		_spi_bus->tx_byte(address & 0xFF);	//ADDR1
		_spi_bus->tx_byte(0);				//ADDR0
		_spi_bus->tx_buf(buffer, 256);
		CS_Line::Set();
		CS_Line::Reset();
		_spi_bus->tx_byte(0x4);	//WRDI
		CS_Line::Set();
	}
	else
		return false;
	return true;
}

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
bool Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::ErasePage(uint32_t page_index)
{
	if(page_index >= PAGE_COUNT) return false;
	if(_spi_bus)
	{
		wait();
		CS_Line::Reset();
		_spi_bus->tx_byte(0x6);	//WREN
		CS_Line::Set();
		CS_Line::Reset();
		const uint16_t address = (page_index * SECTOR_IN_PAGE_COUNT);
		_spi_bus->tx_byte(0x20); //Sector Erase
		_spi_bus->tx_byte(address >> 8);	//ADDR2
		_spi_bus->tx_byte(address & 0xFF);	//ADDR1
		_spi_bus->tx_byte(0);				//ADDR0
		CS_Line::Set();
	}
	else
		return false;
	return true;
}

template <class CS_Line, uint32_t SECTOR_IN_PAGE_COUNT, uint32_t PAGE_COUNT>
void Sst26Memory<CS_Line, SECTOR_IN_PAGE_COUNT, PAGE_COUNT>::wait()
{
	uint8_t status = 0x80;
	do
	{
		CS_Line::Reset();
		_spi_bus->tx_byte(0x5);	//RDSR
		status = _spi_bus->tx_byte(0);
		CS_Line::Set();
	} while (status & 0x80);
}

