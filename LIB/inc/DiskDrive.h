#pragma once
#include <cstdint>

class DiskDrive
{
public:
	virtual uint32_t SectorSize() const = 0;
	virtual uint64_t SectorCount() const = 0;
	virtual bool ReadSector(uint64_t sector_index,uint8_t * buffer)  = 0;
	virtual bool WriteSector(uint64_t sector_index,uint8_t * buffer) = 0;
	virtual bool EraseSector(uint64_t sector_index) = 0;
	virtual void EraseAll() = 0;
};
