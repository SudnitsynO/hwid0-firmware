//#include "pch.h"
#include "FlashDrive.h"

bool FlashDrive::ReadSector(PhysicalSector physical_address, uint8_t* buffer)
{
	return ReadSector(physical_address.physical_page, physical_address.physical_sector, buffer);
}

bool FlashDrive::WriteSector(PhysicalSector physical_address, uint8_t* buffer)
{
	return WriteSector(physical_address.physical_page, physical_address.physical_sector, buffer);
}

uint64_t FlashDrive::SectorNumber(PhysicalSector physical_address)
{
	return physical_address.physical_page * SectorInPageCount() + physical_address.physical_sector;
}

uint64_t FlashDrive::SectorNumber(uint32_t page, uint32_t sector_in_page)
{
	return page * SectorInPageCount() + sector_in_page;
}

uint64_t FlashDrive::TotalSectorCount()
{
	return static_cast<uint64_t>(SectorInPageCount()) * static_cast<uint64_t>(PageCount());
}

PhysicalSector FlashDrive::PhysicalAddres(uint64_t sector_number)
{
	return PhysicalSector{ static_cast<uint32_t>(sector_number / SectorInPageCount()) ,static_cast<uint32_t>(sector_number % SectorInPageCount()) };
}
