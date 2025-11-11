#pragma once
#include "FlashDrive.h"
#include "MutexBase.h"
#include "stm32f1xx_hal.h"
#include <cstdint>
#include <cstring>

template <unsigned long StartAddres_, unsigned long PageCount_, unsigned long SectorSize_, unsigned long PageSize_>
class SystemFlash : public  FlashDrive
{
public:
	//virtual unsigned long SectorSize();
	//virtual unsigned long SectorInPageCount();
	//virtual uint64_t PageCount() ;
	virtual bool ReadSector(uint32_t page_index, uint32_t sector_index, uint8_t * buffer);
	virtual bool WriteSector(uint32_t page_index, uint32_t sector_index, uint8_t * buffer);
	virtual bool ErasePage(uint32_t page_index);
	/*	SystemFlash();
	uint32_t GetSectorSize();
	bool ReadSector(uint8_t * buf, uint32_t sector_number);
	bool WriteSector(void* buf, uint32_t sector_number);
	bool EraseSector(uint32_t sector_number);
	uint32_t GetSectorCount();*/
	void SetLocker(MutexBase &locker);
	 uint32_t SectorSize() const ;
	uint32_t SectorInPageCount() const ;
	 uint64_t PageCount() const ;
private:
	
	MutexBase * _locker;
};

template <unsigned long StartAddres_, unsigned long PageCount_, unsigned long SectorSize_, unsigned long PageSize_>
bool SystemFlash<StartAddres_, PageCount_, SectorSize_, PageSize_>::WriteSector(uint32_t page_index, uint32_t sector_index, uint8_t * buffer)
{
	bool r = true;
	size_t count = 0;
	if (_locker != nullptr) _locker->lock();
	HAL_FLASH_Unlock();
	while (count < SectorSize_)
	{
		FLASH_WaitForLastOperation(1000);
		HAL_FLASH_Program(
			FLASH_TYPEPROGRAM_HALFWORD, StartAddres_ + page_index * PageSize_ + sector_index * SectorSize_ + count,
			*reinterpret_cast<uint16_t*>(buffer + count));
		count += 2;
	}
	HAL_FLASH_Lock();
	if (_locker != nullptr) _locker->release();
	return r;
}

template <unsigned long StartAddres_, unsigned long PageCount_, unsigned long SectorSize_, unsigned long PageSize_>
bool SystemFlash<StartAddres_, PageCount_, SectorSize_, PageSize_>::ReadSector(uint32_t page_index, uint32_t sector_index, uint8_t * buffer)
{
	if (sector_index < SectorInPageCount() && (page_index < PageCount_))
	{
		if (_locker != nullptr) _locker->lock();
		std::memcpy(buffer, (uint8_t*)StartAddres_ + page_index * PageSize_ + sector_index * SectorSize_, SectorSize_);
		if (_locker != nullptr) _locker->release();
		return true;
	}
	return false;
}

template <unsigned long StartAddres_, unsigned long PageCount_, unsigned long SectorSize_, unsigned long PageSize_>
bool SystemFlash<StartAddres_, PageCount_, SectorSize_, PageSize_>::ErasePage(uint32_t page_index)
{
	if (_locker != nullptr) _locker->lock();
	FLASH_EraseInitTypeDef erase_init_type_def = {
		FLASH_TYPEERASE_PAGES,
		FLASH_BANK_1,
		StartAddres_ + page_index * PageSize_,
		1
	};
	uint32_t page_error = 0;
	HAL_FLASH_Unlock();
	bool r = HAL_FLASHEx_Erase(&erase_init_type_def, &page_error) == HAL_OK;
	FLASH_WaitForLastOperation(1000);
	HAL_FLASH_Lock();
	if (_locker != nullptr) _locker->release();
	return r;
}

template <unsigned long StartAddres_, unsigned long PageCount_, unsigned long SectorSize_, unsigned long PageSize_>
void SystemFlash<StartAddres_, PageCount_, SectorSize_, PageSize_>::SetLocker(MutexBase& locker)
{
	_locker = &locker;
}

template <unsigned long StartAddres_, unsigned long PageCount_, unsigned long SectorSize_, unsigned long PageSize_>
uint32_t SystemFlash<StartAddres_, PageCount_, SectorSize_, PageSize_>::SectorSize() const
{
	return SectorSize_;
}

template <unsigned long StartAddres_, unsigned long PageCount_, unsigned long SectorSize_, unsigned long PageSize_>
uint32_t SystemFlash<StartAddres_, PageCount_, SectorSize_, PageSize_>::SectorInPageCount() const
{
	return PageSize_ / SectorSize();
}

template <unsigned long StartAddres_, unsigned long PageCount_, unsigned long SectorSize_, unsigned long PageSize_>
uint64_t SystemFlash<StartAddres_, PageCount_, SectorSize_, PageSize_>::PageCount() const
{
	return PageCount_;
}
