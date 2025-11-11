#pragma once
#include "DiskDrive.h"
#include "FlashDrive.h"
#include <vector>
#include "Cache.h"
#include "Buffer.h"

template <size_t CacheSize>
class FlashTranslator2 : public DiskDrive
{
	struct PageTranslatorRecord
	{
		uint32_t logical_page;
		uint32_t physical_page;
		uint32_t count_of_erase;
	};

	struct SectorTranslatorRecord
	{
		uint32_t logical_sector;
		uint32_t physical_sector;
	};

public:
	FlashTranslator2(FlashDrive* drive);
	virtual uint32_t SectorSize() const override;
	virtual uint64_t SectorCount() const override;
	virtual bool ReadSector(uint64_t sector_index, uint8_t* buffer) override;
	
	virtual bool WriteSector(uint64_t sector_index, uint8_t* buffer) override;
	virtual bool EraseSector(uint64_t sector_index) override;
	virtual void EraseAll() override;

	void init_table();

	uint32_t page_translator_min_size() const; //return page translator size in pages
	uint32_t sector_translator_min_size() const; //return sector translator size in sectors
	uint32_t page_translator_record_count() const; //return count of records page translator in one sector
	uint32_t sector_translator_record_count() const; //return count of records sector translator in one sector
	uint32_t user_page_count() const; //return count of pages contain user data
	uint32_t user_sector_count() const; //return count of sectors in one page contain user data
	PageTranslatorRecord get_ph_page(uint32_t logical_page);
	uint32_t get_ph_sector(uint32_t physical_page, uint32_t logical_sector);
	void swap_page(uint32_t page);
	void page_translator_insert_record(const PageTranslatorRecord record);
	PageTranslatorRecord find_free_page(); //находит свободную страницу с наименьшим износом
	void load();

	void insert_test_record();
	bool ps_invalid(PhysicalSector ps);

	//private: //test
	static constexpr uint32_t ERASED_PAGE = 0xFFFFFFFF;
	static constexpr uint32_t FREE_PAGE = 0xFFFFFFFE;
	static constexpr uint32_t SECTOR_DELETED_P = 0xFFFFFFFF;
	static constexpr uint32_t SECTOR_EMPTY_L = 0xFFFFFFFF;
	FlashDrive* _flash;
	Cache<uint32_t, PageTranslatorRecord, CacheSize> page_translator_cache;
	uint32_t _last_translator_sector; //последний сектор транслятора
	int _last_record_offset; //смещение последней записи
};

template <size_t CacheSize>
FlashTranslator2<CacheSize>::FlashTranslator2(FlashDrive* drive)
	: _flash(drive)
{
}

template <size_t CacheSize>
uint32_t FlashTranslator2<CacheSize>::SectorSize() const
{
	return _flash->SectorSize();
}

template <size_t CacheSize>
uint32_t FlashTranslator2<CacheSize>::page_translator_min_size() const
{
	return (_flash->PageCount() - 1) / (page_translator_record_count() * _flash->SectorInPageCount()) + 1;
}

template <size_t CacheSize>
uint32_t FlashTranslator2<CacheSize>::sector_translator_min_size() const
{
	return ((_flash->SectorInPageCount() * 2) - 1) / sector_translator_record_count() + 1;
}

template <size_t CacheSize>
uint32_t FlashTranslator2<CacheSize>::page_translator_record_count() const
{
	return _flash->SectorSize() / sizeof(PageTranslatorRecord);
}

template <size_t CacheSize>
uint32_t FlashTranslator2<CacheSize>::sector_translator_record_count() const
{
	return _flash->SectorSize() / sizeof(SectorTranslatorRecord);
}

template <size_t CacheSize>
uint32_t FlashTranslator2<CacheSize>::user_page_count() const
{
	return _flash->PageCount() - page_translator_min_size() - 1;
	//резервируем страницы под транслятор и одну для перезаписи
}

template <size_t CacheSize>
uint32_t FlashTranslator2<CacheSize>::user_sector_count() const
{
	return _flash->SectorInPageCount() - sector_translator_min_size();
}

template <size_t CacheSize>
uint64_t FlashTranslator2<CacheSize>::SectorCount() const
{
	return (_flash->SectorInPageCount() - sector_translator_min_size()) * user_page_count();
}

template <size_t CacheSize>
uint32_t FlashTranslator2<CacheSize>::get_ph_sector(uint32_t physical_page, uint32_t logical_sector)
{
	PhysicalSector ps = {physical_page, sector_translator_min_size() - 1};
	Buffer buf(_flash->SectorSize());
	_flash->ReadSector(ps, buf.get());
	auto table = reinterpret_cast<SectorTranslatorRecord*>(buf.get());
	uint32_t result = 0;
	while (result == 0)
	{
		for (int i = sector_translator_record_count() - 1; i >= 0; i--)
		{
			if (table[i].logical_sector == logical_sector)
			{
				result = table[i].physical_sector;
				break;
			}
		}
		if (ps.physical_sector)
		{
			ps.physical_sector--;
			_flash->ReadSector(ps, buf.get());
		}
		else
			break;
	}
	if (ps_invalid({ physical_page,result }) && (result != SECTOR_DELETED_P))
		throw std::logic_error("TRANSLATOR: SECTOR ERROR");
	return result;
}

template <size_t CacheSize>
void FlashTranslator2<CacheSize>::swap_page(uint32_t page)
{
	//сначала ищем незанятую страницу
}

template <size_t CacheSize>
void FlashTranslator2<CacheSize>::page_translator_insert_record(const PageTranslatorRecord record)
{
	Buffer buf(_flash->SectorSize());
	const auto translator = reinterpret_cast<PageTranslatorRecord*>(buf.get());
	buf.memset(0xFF);
	translator[_last_record_offset] = record;
	_flash->WriteSector(_flash->PhysicalAddres(_last_translator_sector), buf.get());
	_last_record_offset++;
	if (_last_record_offset >= page_translator_record_count())
	{
		_last_record_offset = 0;
		_last_translator_sector++;
		if (_last_translator_sector >= page_translator_min_size() * _flash->SectorInPageCount())
		{
			//транслятор переполнен, необходимо перестроить
			std::vector<uint32_t> translator_processing_order; //порядок обработки страниц транслятора
			for (size_t i = 0; i < page_translator_min_size(); i++)
			{
				translator_processing_order.push_back(page_translator_min_size() - i - 1); //задаем текущий торядок
			}
			PageTranslatorRecord free_page = record.logical_page == FREE_PAGE ? record : find_free_page(); //находим пустую страницу для перестановок или используем выданую пользователем
			if (free_page.physical_page == 0) throw std::logic_error("TRANSLATOR:NO FREE PAGE");
			//если не найдена то исключение
			_flash->ErasePage(free_page.physical_page); //очищаем страницу для обмена
			free_page.count_of_erase++;
			//переместим первый сектор транслятора в пустую стр.
			for (size_t i = 0; i < _flash->SectorInPageCount(); i++)
			{
				_flash->ReadSector(0, i, buf.get());
				_flash->WriteSector(free_page.physical_page, i, buf.get());
			}
			translator_processing_order.back() = free_page.physical_page;
			//очищаем транслятор
			_flash->ErasePage(0);
			_last_translator_sector = 0;
			_last_record_offset = 0;
			std::vector<bool> map(_flash->PageCount(), false);
			//запускаем восстановление
			uint32_t page_to_save = 1; //страница транслятора, которую нужно спасти
			for (size_t page = 0; page < translator_processing_order.size(); page++)
			{
				for (int sector = _flash->SectorInPageCount() - 1; sector >= 0; sector--)
				{
					_flash->ReadSector(translator_processing_order[page], sector, buf.get());
					//auto translator = reinterpret_cast<PageTranslatorRecord*>(buf.get());
					for (int record = page_translator_record_count() - 1; record >= 0; record--)
					{
						auto const rec = translator[record];	//извлекаем запись
						if(rec.physical_page > _flash->PageCount()) throw std::logic_error("TRANSLATOR:PAGE TRANSLATOR CORRUPT");
							
						//если инфа о странице еще не обработана,
						if (map[rec.physical_page] == false)
						{
							page_translator_insert_record(rec); //добавляем запись
							map[rec.physical_page] = true; //отметим как обработано
						}
					}
				}
				//страница обработана, очищаем
				_flash->ErasePage(translator_processing_order[page]);
				//нужно спасти следующую страницу транслятора page_to_save
				//ищем, нуждается ли она в обработке
				int index_to_swap = 0; //позиция  в translator_processing_order в которую спаспем
				for (size_t i = page + 1; i < translator_processing_order.size(); i++)
				{
					if (translator_processing_order[i] == page_to_save)
						//если стедующую страницу надо обрабатывать, то сохраним ее в свободную
					{
						index_to_swap = i;
					}
				}
				if (index_to_swap)
				{
					//переместим page_to_save транслятора в недавно стертую стр.
					for (size_t i = 0; i < _flash->SectorInPageCount(); i++)
					{
						_flash->ReadSector(page_to_save, i, buf.get());
						_flash->WriteSector(translator_processing_order[page], i, buf.get());
					}
					_flash->ErasePage(page_to_save); //стираем спасеную
					translator_processing_order[index_to_swap] = translator_processing_order[page];
					//отмечаем в очереди обработки
				}
				page_to_save++;
			}

			/*
			for (size_t page_to_copy = 0; page_to_copy < length; page_to_copy++)
			{

			}
			
			*/
		}
	}
}

template <size_t CacheSize>
typename FlashTranslator2<CacheSize>::PageTranslatorRecord FlashTranslator2<CacheSize>::find_free_page()
{
	auto sector = _last_translator_sector + 1;
	auto offset = _last_record_offset;
	PageTranslatorRecord page{};
	page.count_of_erase = 0;
	page.logical_page = FREE_PAGE;
	page.physical_page = 0;
	uint32_t cer = UINT32_MAX;
	Buffer buf(_flash->SectorSize());
	auto translator = reinterpret_cast<PageTranslatorRecord*>(buf.get());
	std::vector<bool> map(_flash->PageCount(), false);
	while (sector)
	{
		sector--;
		_flash->ReadSector(_flash->PhysicalAddres(sector), buf.get()); //читаем сектор транслятора

		while (offset)
		{
			offset--;
			auto rec = translator[offset];	//читаем одну запись транслятора
			if (rec.physical_page > _flash->PageCount())
				return page;	//запись вне диапазона, повреждение транслятора
			if (map[rec.physical_page] == false)
			{
				if (rec.logical_page == FREE_PAGE)	//если страница свободна
					if (rec.count_of_erase < cer)	//и менее изношена
					{
						cer = rec.count_of_erase;
						page = rec;
					}
				map[rec.physical_page] = true;		//исключаем страницу из поиска
			}
		}
		offset = page_translator_record_count();
	}
	for (size_t i = page_translator_min_size(); i < map.size(); i++)
	{
		if (map[i] == false)
		{
			page.physical_page = i;
			page.logical_page = FREE_PAGE;
			page.count_of_erase = 0;
			break;
		}
	}
	if (page.physical_page == 0)	//пустых страниц нет, такого быть не должно
		throw std::logic_error("TRANSLATOR: NO FREE PAGE");
	if(page.physical_page > _flash->PageCount())
		throw std::logic_error("TRANSLATOR: TABLE ERROR");
	return page;
}

template <size_t CacheSize>
void FlashTranslator2<CacheSize>::load()
{
	//находим начало транслятора
	//найдем последние записи
	uint32_t min_sector = 0;
	uint32_t current_sector = 0;
	uint32_t max_sector = page_translator_min_size() * _flash->SectorInPageCount();
	Buffer buf(_flash->SectorSize());
	for (;;)
	{
		current_sector = (max_sector - min_sector) / 2 + min_sector; //вычислим середину диапазона
		_flash->ReadSector(_flash->PhysicalAddres(current_sector), buf.get());
		const auto r = reinterpret_cast<PageTranslatorRecord*>(buf.get());
		if (r->physical_page == 0xFFFFFFFF)
		{
			max_sector = current_sector;
		}
		else
		{
			min_sector = current_sector;
		}
		if ((max_sector - min_sector) <= 1)
			break;
	}
	_last_translator_sector = min_sector;
	_flash->ReadSector(_flash->PhysicalAddres(_last_translator_sector), buf.get());
	const auto r = reinterpret_cast<PageTranslatorRecord*>(buf.get());
	//поиск хвоста
	int offset = 0;
	for (offset = page_translator_record_count() - 1; offset >= 0; offset--)
	{
		if (r[offset].physical_page != 0xFFFFFFFF)
			break;
	}
	offset++;
	if (offset == page_translator_record_count())
	{
		_last_record_offset = 0;
		_last_translator_sector++;
	}
	else
		_last_record_offset = offset;
}

template <size_t CacheSize>
void FlashTranslator2<CacheSize>::insert_test_record()
{
	PageTranslatorRecord r;
	r.physical_page = _flash->PageCount() - 1;
	r.logical_page = FREE_PAGE;
	r.count_of_erase = 0;
	page_translator_insert_record(r);
}

template <size_t CacheSize>
bool FlashTranslator2<CacheSize>::ps_invalid(PhysicalSector ps)
{
	if(ps.physical_page >= _flash->PageCount())
		return true;
	if(ps.physical_sector >= _flash->SectorInPageCount())
		return true;
	return false;
}

template <size_t CacheSize>
typename FlashTranslator2<CacheSize>::PageTranslatorRecord FlashTranslator2<CacheSize>::get_ph_page(
	uint32_t logical_page)
{
	auto it = page_translator_cache.find(logical_page);
	if (it != page_translator_cache.end())
	{
		//найдено в кэш
		return page_translator_cache[it];
	}
	else
	{
		//будем искать в трансляторе
		/*
		
		*/
		Buffer buf(_flash->SectorSize());
		auto sector = _last_translator_sector + 1;
		auto offset = _last_record_offset;
		bool result = false;
		PageTranslatorRecord rec {};
		auto ptr = reinterpret_cast<PageTranslatorRecord*>(buf.get());
		while (sector)
		{
			sector--;
			_flash->ReadSector(_flash->PhysicalAddres(sector), buf.get());
			while (offset)
			{
				offset--;
				if (ptr[offset].logical_page == logical_page)
				{
					result = true;
					rec = ptr[offset];
					sector = 0;
					break;
				}
			}
			offset = page_translator_record_count();
		}
		if (!result)
		{
			rec.physical_page = 0;
			rec.logical_page = logical_page;
		}
		page_translator_cache.add(logical_page, rec);
		if (ps_invalid({ rec.physical_page,0 })) throw std::logic_error("TRANSLATOR: PAGE ERROR");
		return rec;
	}
}

template <size_t CacheSize>
bool FlashTranslator2<CacheSize>::ReadSector(uint64_t sector_index, uint8_t* buffer)
{
	if (sector_index >= SectorCount()) return false;
	//разбиваем sector_index
	const uint32_t logic_page = sector_index / user_sector_count();
	const uint32_t logic_sector = sector_index % user_sector_count();
	PhysicalSector ps;
	//поиск страницы
	ps.physical_page = get_ph_page(logic_page).physical_page;
	if (ps.physical_page == 0) return false; //нет такой страницы
	ps.physical_sector = get_ph_sector(ps.physical_page, logic_sector);
	if (ps.physical_sector == 0) return false; //нет упоминаний
	if (ps.physical_sector == SECTOR_DELETED_P) return false; //сектор удален
	_flash->ReadSector(ps, buffer);
	return true;
}

template <size_t CacheSize>
bool FlashTranslator2<CacheSize>::WriteSector(uint64_t sector_index, uint8_t* buffer)
{
	if (sector_index >= SectorCount()) return false;
	Buffer buf(_flash->SectorSize());
	auto table = reinterpret_cast<SectorTranslatorRecord*>(buf.get());
	if (table == nullptr) return false;

	//разбиваем sector_index
	const uint32_t logic_page = sector_index / user_sector_count();
	const uint32_t logic_sector = sector_index % user_sector_count();

	PhysicalSector ps;
	//поиск страницы
	PageTranslatorRecord current_page = get_ph_page(logic_page);
	ps.physical_page = current_page.physical_page;
	if (ps.physical_page == 0)
	{
		//такой страницы нет в трансляторе
		//выделим новую
		auto free_page = find_free_page();
		//стираем
		_flash->ErasePage(free_page.physical_page);
		++free_page.count_of_erase;
		free_page.logical_page = logic_page;
		page_translator_insert_record(free_page);
		page_translator_cache.add(logic_page, free_page);
		ps.physical_page = free_page.physical_page;
	}

	ps.physical_sector = sector_translator_min_size();
	//будем искать послений занятый сектор
	uint32_t sector_to_write = 0;
	for (size_t i = 0; i < sector_translator_min_size(); i++)
	{
		--ps.physical_sector;
		_flash->ReadSector(ps, buf.get()); //читаем сектор транслятора страницы (начиная с последнего)
		for (int offset = sector_translator_record_count() - 1; offset >= 0; offset--)
		{
			if (table[offset].physical_sector != SECTOR_DELETED_P)
			{
				sector_to_write = table[offset].physical_sector + 1;
				if (sector_to_write > _flash->SectorInPageCount())
					return false;	//ошибка транслятора
				break;
			}
		}
		if (sector_to_write) break;
	}
	if (sector_to_write < sector_translator_min_size())
	{
		//записей в трансляторе нету
		sector_to_write = sector_translator_min_size(); //запишем в первый сектор
	}
	//sector_to_write++;
	if (sector_to_write >= _flash->SectorInPageCount())
	{
		//страница заполнена, переносим
		sector_to_write = sector_translator_min_size();
		std::vector<bool> map(user_sector_count(), false); //карта перенесеных секторов
		map[logic_sector] = true;
		auto free_page = find_free_page(); //находим пустую страницу
		//стираем
		_flash->ErasePage(free_page.physical_page);
		++free_page.count_of_erase;
		free_page.logical_page = logic_page;
		//uint32_t sector_to_write = 0;
		auto table_sector = sector_translator_min_size();
		uint32_t target_table_offset = 0;
		uint32_t target_table_sector = 0;
		uint32_t target_sector = sector_translator_min_size();
		for (size_t i = 0; i < sector_translator_min_size(); i++)
		{
			--table_sector;
			ps.physical_sector = table_sector;
			_flash->ReadSector(ps, buf.get());
			for (int offset = sector_translator_record_count() - 1; offset >= 0; offset--)
			{
				if (table[offset].logical_sector != SECTOR_EMPTY_L)
				{
					auto ls = table[offset].logical_sector;
					if(ls >= map.size())
						return false;	//ошибка транслятора
					if (map[ls] == false)
					{
						if (table[offset].physical_sector != SECTOR_DELETED_P)
							//если этого сектора еще небыло и он не удален
						{
							//переносим
							Buffer buf2(_flash->SectorSize());
							auto table2 = reinterpret_cast<SectorTranslatorRecord*>(buf2.get());
							ps.physical_sector = table[offset].physical_sector;//грязные данные
							if(ps_invalid(ps)) return false;	//ошибка транслятора
							_flash->ReadSector(ps, buf2.get());
							_flash->WriteSector(free_page.physical_page, target_sector, buf2.get());
							//запись в таблицу
							buf2.memset(0xFF);
							table2[target_table_offset].physical_sector = target_sector;
							table2[target_table_offset].logical_sector = table[offset].logical_sector;
							_flash->WriteSector(free_page.physical_page, target_table_sector, buf2.get());	//делаем одну запись в новом трансляторе сектора
							target_table_offset++;
							target_sector++;
							sector_to_write = target_sector;//????
							if (target_table_offset >= sector_translator_record_count())	//текущий сектор транслятора заполнен
							{
								target_table_offset = 0;
								target_table_sector++;
							}
							map[ls] = true;
						}
						else
						{
							map[ls] = true;
						}
					}
				}
			}
		}
		// регистрация новой страницы
		//отмечаем текущую как свободную
		current_page.logical_page = FREE_PAGE;
		page_translator_insert_record(current_page);
		//отмечаем новую как логическую
		free_page.logical_page = logic_page;
		page_translator_insert_record(free_page);
		page_translator_cache.add(logic_page, free_page);
		current_page = free_page;
		ps.physical_page = current_page.physical_page;
	}

	//находим пустое место в таблице
	uint32_t last_empty_record_sector = 0;
	uint32_t last_empty_record_offset = 0;
	ps.physical_sector = sector_translator_min_size();
	for (size_t i = 0; i < sector_translator_min_size(); i++)
	{
		--ps.physical_sector;
		_flash->ReadSector(ps, buf.get());
		for (int offset = sector_translator_record_count() - 1; offset >= 0; offset--)
		{
			if (table[offset].logical_sector != SECTOR_EMPTY_L)
			{
				//last_empty_record_sector = offset;
				i = sector_translator_min_size();
				break;
			}
			else
			{
				last_empty_record_sector = ps.physical_sector;
				last_empty_record_offset = offset;
			}
		}
	}
	//делаем запись в таблице
	buf.memset(0xFF);
	table[last_empty_record_offset].logical_sector = logic_sector;
	table[last_empty_record_offset].physical_sector = sector_to_write;
	ps.physical_sector = last_empty_record_sector;
	_flash->WriteSector(ps, buf.get());
	ps.physical_sector = sector_to_write;
	_flash->WriteSector(ps, buffer);
	return true;
}

template <size_t CacheSize>
bool FlashTranslator2<CacheSize>::EraseSector(uint64_t sector_index)
{
	if (sector_index >= SectorCount()) return false;
	//разбиваем sector_index
	const uint32_t logic_page = sector_index / user_sector_count();
	const uint32_t logic_sector = sector_index % user_sector_count();
	PhysicalSector ps;
	//поиск страницы
	ps.physical_page = get_ph_page(logic_page).physical_page;
	if (ps.physical_page == 0) return false; //нет такой страницы
	ps.physical_sector = get_ph_sector(ps.physical_page, logic_sector);
	if (ps.physical_sector == 0) return false; //нет упоминаний
	if (ps.physical_sector == SECTOR_DELETED_P) return false; //сектор удален

	Buffer buf(_flash->SectorSize());
	auto table = reinterpret_cast<SectorTranslatorRecord*>(buf.get());

	//находим пустое место в таблице
	uint32_t last_empty_record_sector = 0;
	uint32_t last_empty_record_offset = 0;
	ps.physical_sector = sector_translator_min_size();
	for (size_t i = 0; i < sector_translator_min_size(); i++)
	{
		--ps.physical_sector;
		_flash->ReadSector(ps, buf.get());
		for (int offset = sector_translator_record_count() - 1; offset >= 0; offset--)
		{
			if (table[offset].logical_sector != SECTOR_EMPTY_L)
			{
				//last_empty_record_sector = offset;
				i = sector_translator_min_size();
				break;
			}
			else
			{
				last_empty_record_sector = ps.physical_sector;
				last_empty_record_offset = offset;
			}
		}
	}
	//делаем запись в таблице
	buf.memset(0xFF);
	table[last_empty_record_offset].logical_sector = logic_sector;
	table[last_empty_record_offset].physical_sector = SECTOR_DELETED_P;
	ps.physical_sector = last_empty_record_sector;
	_flash->WriteSector(ps, buf.get());
	return true;

	return true;
}

template <size_t CacheSize>
void FlashTranslator2<CacheSize>::EraseAll()
{
	init_table();	//TODO: переписать для сохренения данных об износе страниц
}

template <size_t CacheSize>
void FlashTranslator2<CacheSize>::init_table()
{
	//стираем таблицы, в которых расположен транслятор
	for (size_t i = 0; i < page_translator_min_size(); i++)
	{
		_flash->ErasePage(i);
	}
	//сброс кэша транслятора страниц
	page_translator_cache.reset();
	//отметим все страницы как свободные
	_last_translator_sector = 0;
	_last_record_offset = 0;
}
