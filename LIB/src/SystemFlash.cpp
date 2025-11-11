#include "SystemFlash.h"
#include <string.h>


void Unlock()
{
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
}

void Lock()
{
	FLASH->CR |= FLASH_CR_LOCK;
}

bool Ready()
{
	return !(FLASH->SR & FLASH_SR_BSY);
}

void ErasePage(void* addr)
{
	FLASH->CR |= FLASH_CR_PER; //Устанавливаем бит стирания одной страницы
	FLASH->AR = (uint32_t)addr; // Задаем её адрес
	FLASH->CR |= FLASH_CR_STRT; // Запускаем стирание 
	while (!Ready());  //Ждем пока страница сотрется. 
	FLASH->CR &= ~FLASH_CR_PER; //Сбрасываем бит обратно
}

void Write(uint32_t address, uint32_t data)
{
	// ReSharper disable once CppPossiblyErroneousEmptyStatements
	while (!Ready()); //Ожидаем готовности флеша к записи
	FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша
	*(__IO uint16_t*)address = (uint16_t)data; //Пишем младшие 2 байта
	while (!Ready());
	address += 2;
	data >>= 16;
	*(__IO uint16_t*)address = (uint16_t)data; //Пишем старшие 2 байта
	while (!Ready());
	FLASH->CR &= ~(FLASH_CR_PG); //Запрещаем программирование флеша
}


SystemFlash::SystemFlash(uint32_t start_addres, uint32_t sector_count, uint32_t sector_size)
{
	_sector_count = sector_count;
	_start_addres = start_addres;
	_sector_size = sector_size;
	_locker = NULL;
}

uint32_t SystemFlash::GetSectorSize()
{
	return _sector_size;
}

bool SystemFlash::ReadSector(uint8_t* buf, uint32_t sector_number)
{
	if (sector_number < _sector_count)
	{
		if(_locker != NULL) _locker->Lock();
		memcpy(buf, (uint8_t*)_start_addres + _sector_size * sector_number, _sector_size);
		if (_locker != NULL) _locker->Release();
		return true;
	}
	else
		return false;
}

bool SystemFlash::WriteSector(void* buf, uint32_t sector_number)
{
	uint32_t * data = (uint32_t*)buf;
	if (sector_number < _sector_count)
	{
		if (_locker != NULL) _locker->Lock();
		Unlock();
//		ErasePage((PVOID)(_start_addres + _sector_size * sector_number));
		for (int i = 0; i < _sector_size / 4; i++)
		{
			Write(_start_addres + _sector_size * sector_number + i * 4, data[i]);
		}
		Lock();
		if (_locker != NULL) _locker->Release();
		return true;
	}
	return false;

}

bool SystemFlash::EraseSector(uint32_t sector_number)
{
	if (sector_number < _sector_count)
	{
		if (_locker != NULL) _locker->Lock();
		Unlock();
		ErasePage((void*)(_start_addres + _sector_size * sector_number));
		Lock();
		if (_locker != NULL) _locker->Release();
		return true;
	}
	return false;
}

uint32_t SystemFlash::GetSectorCount()
{
	return _sector_count;
}

void SystemFlash::SetLocker(MutexBase* locker)
{
	_locker = locker;
}
