#pragma once
#include <cstdint>

struct PhysicalSector
{
	uint32_t physical_page;
	uint32_t physical_sector;
};

class FlashDrive
{
public:
	virtual uint32_t SectorSize() const = 0;			//return sector size in bytes
	virtual uint32_t SectorInPageCount() const  = 0;	//return page size in sectors
	virtual uint64_t PageCount() const = 0;				//page count in flash drive

	virtual bool ReadSector(uint32_t page_index, uint32_t sector_index, uint8_t * buffer) = 0;	//read one sector
	virtual bool ReadSector(PhysicalSector physical_address, uint8_t * buffer);	//read one sector
	virtual bool WriteSector(uint32_t page_index, uint32_t sector_index, uint8_t * buffer) = 0;	//write one sector
	virtual bool WriteSector(PhysicalSector physical_address, uint8_t * buffer);	//write one sector
	virtual bool ErasePage(uint32_t page_index) = 0;											//erase one page
	uint64_t SectorNumber(PhysicalSector physical_address);
	uint64_t SectorNumber(uint32_t page,uint32_t sector_in_page);
	uint64_t  TotalSectorCount();
	PhysicalSector PhysicalAddres(uint64_t sector_number);
};
