//#include "pch.h"
#include "FlashTranslator.h"
#include "random.h"
//#include <iostream>
#include <cstring>


//actions in the translator record (physical page field)
#define FREE_SECTOR (0xFFFFFFFFUL)    //user delete logic sector (in page filed)
#define ERASE_PAGE (0xFFFFFFFFUL)    //flash memory page erase (in sector filed)
#define EMPTY_RECORD (0xFFFFFFFFFFFFFFFFULL)   //empty translator record;

XorShift rnd1;

SpaceEntry::SpaceEntry(uint64_t begin_, uint64_t end_) : begin(begin_), end(end_)
{
}

bool SpaceEntry::operator<(const SpaceEntry& foo) const
{
	return begin < foo.begin;
}

FlashTranslator::FlashTranslator() : _flash(nullptr)
{//constructor
}

void FlashTranslator::Init(FlashDrive& flash, int max_fragments)
{
	_flash = &flash;
	logic_sector_count = flash.SectorSize()*flash.SectorInPageCount()*flash.PageCount();
	records_in_sector = (flash.SectorSize() - sizeof(SectorDescriptor)) / sizeof(TranslatorEntry);
	logic_sector_count = logic_sector_count - (logic_sector_count / records_in_sector + 1);
	_max_fragments = max_fragments;
}



uint32_t FlashTranslator::SectorSize() const 
{
	if (_flash == nullptr)  return 0;
	return _flash->SectorSize() - sizeof(uint64_t);
}

uint64_t FlashTranslator::SectorCount() const 
{
	if (_flash == nullptr)  return 0;
	return  logic_sector_count;
}

bool FlashTranslator::ReadSector(uint64_t sector_index, uint8_t* buffer)
{
	if (_flash == nullptr)  return false;
	if (sector_index >= logic_sector_count) return false;
	auto it = table.find(sector_index);
	if (it == table.end()) return false;
	auto buf = new uint8_t[_flash->SectorSize()];
	if (buf == nullptr) return false;
	auto r = _flash->ReadSector(it->second.physical_page, it->second.physical_sector, buf);
	if (r)
		std::memcpy(buffer, buf + sizeof(uint64_t), SectorSize());
	delete[] buf;
	return r;
}

void FlashTranslator::Defrag(const uint32_t last_free_page)
{
	//std::cout << "D" << std::endl;
	if (_flash == nullptr)  return;
	auto buf = new uint8_t[_flash->SectorSize()];
	if (buf == nullptr) return;
	
	auto empty_page = last_free_page;
	_flash->ErasePage(empty_page);
	AddSpice(erased_space,
	         SpaceEntry(empty_page * _flash->SectorInPageCount(), (empty_page + 1) * _flash->SectorInPageCount() - 1));
	//free translator space
	SectorDescriptor translator_sector_descriptor{ 0,0 };
	AddSpice(free_space, 0);
	for(;;)
	{
		
		_flash->ReadSector(translator_sector_descriptor.next_page, translator_sector_descriptor.next_sector,buf);
		translator_sector_descriptor = *reinterpret_cast<SectorDescriptor *>(buf);						//get sector descriptor
		if (translator_sector_descriptor.next_page == 0xFFFFFFFF) 
			break;
		AddSpice(free_space, _flash->SectorNumber(translator_sector_descriptor.next_page,translator_sector_descriptor.next_sector));
	}
	//defrag begin edges of free space blocks
	uint32_t empty_sector = 0;
	SpaceEntry current_free_block = *free_space.begin();
	uint64_t start_of_block = 0;		//inclusive
	uint64_t end_of_block = 0;        	//not inclusive
	bool to_break = false;
	for (;;)
	{
		
		end_of_block = current_free_block.begin;// not inclusive
		
		if (start_of_block < end_of_block)
		{
			end_of_block--;		//make inclusive
			//zone of dada
			PhysicalSector first_sector = _flash->PhysicalAddres(start_of_block);
			PhysicalSector last_sector = _flash->PhysicalAddres(end_of_block);
			
			uint32_t page = first_sector.physical_page;
			uint32_t start_sector = first_sector.physical_sector;
			uint32_t end_sector = last_sector.physical_page == page ? last_sector.physical_sector : _flash->SectorInPageCount() - 1;
			for (;;)
			{
				if (page != empty_page)
					for (uint32_t sector = start_sector; sector <= end_sector; sector++)
					{
						_flash->ReadSector(page, sector, buf);
						AddSpice(free_space, _flash->SectorNumber(page, sector));
						auto logic_sector = *reinterpret_cast<uint64_t*>(buf);
						_flash->WriteSector(empty_page, empty_sector, buf);
						table[logic_sector] = PhysicalSector{ empty_page,empty_sector };
						CutSpice(free_space, _flash->SectorNumber(empty_page, empty_sector));
						CutSpice(erased_space, _flash->SectorNumber(empty_page, empty_sector));
						empty_sector++;
						if (empty_sector == _flash->SectorInPageCount())
						{
							empty_sector = 0;
							empty_page = FindEmptyPage();
							if (empty_page != 0)
							{
								_flash->ErasePage(empty_page);
								AddSpice(erased_space, SpaceEntry(empty_page * _flash->SectorInPageCount(), (empty_page + 1) * _flash->SectorInPageCount() - 1));
							}
							else
								for (;;);
						}
					}
				if(page == last_sector.physical_page) break;
				page = last_sector.physical_page;
				start_sector = 0;
				end_sector = last_sector.physical_sector;
			}
		}
		if(to_break)
			break;
		start_of_block = current_free_block.end + 1;	//inclusive
		auto it = free_space.upper_bound(current_free_block);
		if(it == free_space.end())
		{
			current_free_block.begin = _flash->TotalSectorCount();
			current_free_block.end = _flash->TotalSectorCount();
			to_break = true;
		}
		else
		{
			current_free_block = *it;
		}
	}
	//recovery translator structure
	_flash->ErasePage(0);
	AddSpice(erased_space, SpaceEntry(0, _flash->SectorInPageCount() - 1));
	translator_index = 0;
	translator_page = 0;
	translator_sector = 0;
	current_sector_descriptor.next_page = 0;
	current_sector_descriptor.next_sector = 1;
	std::memset(buf, 0xFF, _flash->SectorSize());
	*reinterpret_cast<SectorDescriptor*>(buf) = current_sector_descriptor;
	_flash->WriteSector(0, 0, buf);
	CutSpice(free_space, 0);
	CutSpice(free_space, 1);
	CutSpice(erased_space, 0);
	CutSpice(erased_space, 1);
	delete[] buf;
	/*
	for (auto i : erased_space)
	{
		auto begin_ps = _flash->PhysicalAddres(i.begin);
		auto end_ps = _flash->PhysicalAddres(i.end);
		for (uint32_t p = begin_ps.physical_page; p <= end_ps.physical_page;p++)
		{
			TranslatorEntry entry;
			entry.logic_sector = 0;
			entry.physical_address.physical_sector = ERASE_PAGE;
			entry.physical_address.physical_page = p;
			if (p != 0)
				InsertTranslatorRecord(entry);
		}
	}
	*/
	for (auto i : table)
	{
			TranslatorEntry entry;
			entry.logic_sector = i.first;
			entry.physical_address = i.second;
				InsertTranslatorRecord(entry);
	}
}

uint64_t FlashTranslator::GetEmptySector()
{
	if (erased_space.empty())
	{
		auto candidate_block = free_space.upper_bound(SpaceEntry(rnd1.random64(_flash->TotalSectorCount()), 0));
		if (candidate_block != free_space.begin())
			--candidate_block;
		if (candidate_block == free_space.end())
			candidate_block = free_space.begin();
		auto it = candidate_block;
		uint32_t page1 = 0;
		uint32_t page2 = 0;
		while  (page2 == 0)
		{
			
			auto sector = it->begin + _flash->SectorInPageCount() - 1;
			auto start_page = sector / _flash->SectorInPageCount();
			auto end_page = (it->end + 1) / _flash->SectorInPageCount();
			if(page1 == 0)
				if (start_page < end_page)
				{
					page1 = start_page;
					start_page++;
				}
			if (start_page < end_page)
			{
				page2 = start_page;
			}
			
			++it;
			if (it == free_space.end())
				it = free_space.begin();
			if (it == candidate_block)
				break;
		}
		if (page1 && page2)
		{
			//erase start page
			_flash->ErasePage(page1);
			AddSpice(erased_space, SpaceEntry(page1 * _flash->SectorInPageCount(), (page1 + 1) * _flash->SectorInPageCount() - 1));
			//translator update
			TranslatorEntry entry;
			entry.logic_sector = 0;
			entry.physical_address.physical_sector = ERASE_PAGE;
			entry.physical_address.physical_page = page1;
			InsertTranslatorRecord(entry);
		}
		else if (page1)
			Defrag(page1);

	}
	return erased_space.begin()->begin;
}

uint32_t FlashTranslator::FindEmptyPage()
{
	for (auto i : free_space)
	{
		auto sector = i.begin + _flash->SectorInPageCount() - 1;
		auto start_page = sector / _flash->SectorInPageCount();
		auto end_page = (i.end + 1) / _flash->SectorInPageCount();
		if (start_page == 0) start_page++;
		if (start_page < end_page)
			return start_page;
	}
	return 0;
}

bool FlashTranslator::WriteSector(uint64_t sector_index, uint8_t* buffer)
{
	
	if (_flash == nullptr)  return false;
	if (free_space.size() > _max_fragments)
		Defrag(FindEmptyPage());
	if (sector_index >= logic_sector_count) return false;
	auto buf = new uint8_t[_flash->SectorSize()];
	if (buf == nullptr) return false;
	auto sector_number = GetEmptySector();
	auto new_ps = _flash->PhysicalAddres(sector_number);
	if(sector_number == 0)	//no space
		return false;
	auto old_place = table.find(sector_index);
	if(old_place!= table.end())	//if old place found
	{
		AddSpice(free_space, _flash->SectorNumber(old_place->second));	//free old sector place
		//TranslatorEntry entry;
		//entry.logic_sector = sector_index;
		//entry.physical_address.physical_sector = 0;
		//entry.physical_address.physical_page = FREE_SECTOR;
		//InsertTranslatorRecord(entry);									//free old block in the translator
	}
	CutSpice(free_space, sector_number);	//mark new sector place is busy
	CutSpice(erased_space, sector_number);  //mark new sector place is not erased
	TranslatorEntry entry;
	entry.logic_sector = sector_index;
	entry.physical_address.physical_sector = new_ps.physical_sector;
	entry.physical_address.physical_page = new_ps.physical_page;
	InsertTranslatorRecord(entry);				//insert new translator record						
	
	table[sector_index] = _flash->PhysicalAddres(sector_number);  //update translator record
	*reinterpret_cast<uint64_t*>(buf) = sector_index;
	std::memcpy(buf + sizeof(uint64_t), buffer, SectorSize());
	const auto r = _flash->WriteSector(entry.physical_address.physical_page, entry.physical_address.physical_sector, buf);
	delete[] buf;
	return r;
}

void FlashTranslator::Rebuild()
{
	if (_flash == nullptr)  return;
	table.clear();
	const auto buf = new uint8_t[_flash->SectorSize()];
	if (buf)
	{
		std::memset(buf, 0xFF, _flash->SectorSize());
		const auto descriptor = reinterpret_cast<SectorDescriptor*>(buf);
		//auto entries = reinterpret_cast<TranslatorEntry*>(buf + sizeof(SectorDescriptor));
			descriptor->next_sector = 1;
			descriptor->next_page = 0;
			_flash->ErasePage(0);
			_flash->WriteSector(0, 0, buf);
		//} while ((descriptor->next_sector == 0) && (descriptor->next_page == 0));
	}
	delete[] buf;
}

bool FlashTranslator::EraseSector(uint64_t sector_index)
{
	auto old_pos = table.find(sector_index);
	if (old_pos != table.end())
	{
		TranslatorEntry entry;
		entry.logic_sector = sector_index;
		entry.physical_address.physical_page = FREE_SECTOR;
		entry.physical_address.physical_sector = 0;
		InsertTranslatorRecord(entry);

		AddSpice(free_space, _flash->SectorNumber(old_pos->second));
		table.erase(old_pos);

		return true;
	}
	return false;
}

void FlashTranslator::EraseAll()
{
	Rebuild();
}

bool FlashTranslator::ReadTable()
{
	table.clear();
	free_space.clear();
	erased_space.clear();
	erased_space.insert(SpaceEntry(0,_flash->SectorInPageCount()-1));
	free_space.insert(SpaceEntry( 0,_flash->TotalSectorCount() - 1 ));
	const auto buf = new uint8_t[_flash->SectorSize()];
	translator_page = 0;
	translator_sector = 0;
	translator_index = 0;
	if (buf)
	{
		for (;;)	//read sectors
		{
			if ((translator_sector < _flash->SectorInPageCount() && (translator_page < _flash->PageCount())))	//check range
			{
				CutSpice(erased_space, _flash->SectorNumber(translator_page, translator_sector));
				CutSpice(free_space, _flash->SectorNumber(translator_page, translator_sector));
				if (_flash->ReadSector(translator_page,translator_sector, buf))								//read sector data
				{
					current_sector_descriptor = *reinterpret_cast<SectorDescriptor *>(buf);						//get sector descriptor
					const auto entries = reinterpret_cast<TranslatorEntry *>(buf + sizeof(SectorDescriptor));	//get entries link
					while (translator_index < records_in_sector)												//read entries
					{
						if (entries[translator_index].logic_sector == EMPTY_RECORD)						//if empty entry
							break;
						if(entries[translator_index].physical_address.physical_page == FREE_SECTOR)				//user clear logic sector
						{
							auto it = table.find(entries[translator_index].logic_sector);
							if (it != table.end())
							{
								AddSpice(free_space,_flash->SectorNumber(it->second) );
								table.erase(it);
							}
						}
						else if(entries[translator_index].physical_address.physical_sector == ERASE_PAGE)            //erase page
						{
							const auto base = _flash->SectorInPageCount() * entries[translator_index].physical_address.physical_page;
							AddSpice(erased_space, SpaceEntry(base, _flash->SectorInPageCount() + base - 1));
						}
						else																					//user write sector
						{
							auto it = table.find(entries[translator_index].logic_sector);											//get old physical address
							if(it != table.end())
							{
								AddSpice(free_space, _flash->SectorNumber(it->second));
							}
							table[entries[translator_index].logic_sector] = entries[translator_index].physical_address;				//save translator record
							CutSpice(free_space,   _flash->SectorNumber(entries[translator_index].physical_address));//cut sector from free space map
							CutSpice(erased_space, _flash->SectorNumber(entries[translator_index].physical_address));//cut sector from erased space map
						}
						translator_index++;
					}
					if (translator_index == records_in_sector)													//if need next sector
					{
						translator_page = current_sector_descriptor.next_page;
						translator_sector = current_sector_descriptor.next_sector;
						current_sector_descriptor.next_page = 0xFFFFFFFF;
						current_sector_descriptor.next_sector = 0xFFFFFFFF;
						translator_index = 0;
					}
					else
						break;																					// break sector cycle
				}
			}
			
		}
		auto next_translator_sector_number = _flash->SectorNumber(current_sector_descriptor.next_page, current_sector_descriptor.next_sector);
		CutSpice(erased_space, next_translator_sector_number);
		CutSpice(free_space, next_translator_sector_number);

	}
	delete[] buf;
	return true;
}
/*
void FlashTranslator::Test()
{
	free_space.clear();
	erased_space.clear();
	
	AddSpice(free_space, SpaceEntry(11,19));
	AddSpice(free_space, SpaceEntry(25, 400));
	
	//AddSpice(free_space, SpaceEntry(25, 26));
	GetEmptySector();

	for (auto i : erased_space)
	{
		std::cout << "{" << i.begin << "-" << i.end << "}" << std::endl;
	}
	std::cout << endl;
	
	GetEmptySector();

	for (auto i : erased_space)
	{
		std::cout << "{" << i.begin << "-" << i.end << "}" << std::endl;
	}
	std::cout << endl;
	
	AddSpice(free_space, SpaceEntry(15, 20));
	for (auto i : free_space)
	{
		std::cout << "{" << i.begin << "-" << i.end << "}" << std::endl;
	}
	std::cout << endl;
	AddSpice(free_space, SpaceEntry(15, 26));
	for (auto i : free_space)
	{
		std::cout << "{" << i.begin << "-" << i.end << "}" << std::endl;
	}
	std::cout << endl;
	AddSpice(free_space, SpaceEntry(15, 26));
	for (auto i : free_space)
	{
		std::cout << "{" << i.begin << "-" << i.end << "}" << std::endl;
	}
	std::cout << endl;
	AddSpice(free_space, SpaceEntry(5, 35));
	for (auto i : free_space)
	{
		std::cout << "{" << i.begin << "-" << i.end << "}" << std::endl;
	}
	std::cout << endl;
	AddSpice(free_space, SpaceEntry(5, 100));

	for (auto i : free_space)
	{
		std::cout << "{" << i.begin << "-" << i.end << "}" << std::endl;
	}
	std::cout << endl;
	
	*/

//}

void FlashTranslator::CutSpice(set<SpaceEntry>& map, uint64_t sector_number)
{
	if(map.empty()) return;
	if (!SectorInMap(sector_number, map)) return;
	auto it = map.upper_bound(SpaceEntry{ sector_number ,0});
	--it;
	SpaceEntry entry = *it;
	map.erase(it);
	if(sector_number == entry.begin)
	{
		entry.begin++;
		if (entry.begin <= entry.end)
			map.insert(entry);
	}
	else if (sector_number == entry.end)
	{
		entry.end--;
		if (entry.end >= entry.begin)
			map.insert(entry);
	}
	else
	{
		map.insert(SpaceEntry{entry.begin,sector_number-1});
		map.insert(SpaceEntry{sector_number + 1, entry.end});
	}
}

void FlashTranslator::AddSpice(set<SpaceEntry>& map, uint64_t sector_number)
{
	if (sector_number == 4294967264)
		for (;;);
	if(map.empty())
	{
		map.insert(SpaceEntry(sector_number, sector_number));
	}
	else
	{
		if (SectorInMap(sector_number, map)) return;
		auto it = map.upper_bound(SpaceEntry{ sector_number ,0 });
		if (it == map.begin()) //previous block not found
		{
			auto next_block = *it;
			if ((next_block.begin - 1) == sector_number)  //add to next
			{
				map.erase(it);
				map.insert(SpaceEntry(sector_number, next_block.end));
			}
			else //add to new
			{
				map.insert(SpaceEntry{ sector_number,sector_number });
			}
		}
		else if (it == map.end())//next block not found
		{
			auto previous_block = *(--it);
			if ((previous_block.end + 1) == sector_number)	//add to previous
			{
				map.erase(it);
				map.insert(SpaceEntry(previous_block.begin, sector_number));
			}
			else //add to new
			{
				map.insert(SpaceEntry{ sector_number,sector_number });
			}
		}
		else   //full processing
		{
			auto next_block = *it--;
			auto previous_block = *it;
			if ((previous_block.end + 1) == sector_number)	//add to previous
			{
				if ((next_block.begin - 1) == sector_number)	//gluing
				{
					it = map.erase(it);	//delete previous
					map.erase(it);	    //delete next
					map.insert(SpaceEntry(previous_block.begin, next_block.end));
				}
				else
				{
					map.erase(it);
					map.insert(SpaceEntry(previous_block.begin, sector_number));
				}
			}
			else if ((next_block.begin - 1) == sector_number) //add to next
			{
				map.erase(++it);
				map.insert(SpaceEntry{ sector_number,next_block.end });
			}
			else //create new
			{
				map.insert(SpaceEntry{ sector_number,sector_number });
			}
		}
	}
}

void FlashTranslator::AddSpice(set<SpaceEntry>& map, SpaceEntry area)
{
	if (map.empty())
	{
		map.insert(area);
	}
	else
	{
		auto it = map.upper_bound(area);
		if (it != map.begin()) //previous block found
		{
			--it;				//go to previous
			if (it->end >= area.begin)	//previous block crossing with 'area'
			{
				area.begin = it->begin; // add previous block space to 'area'
				it = map.erase(it);		//delete previous block
			}
			else
				++it;
		}
		bool completed = false;
		while ((it!= map.end()) && !completed)
		{
			if (it->begin > area.end)
				break;
			if(it->end >= area.end)	//last block detected
			{
				area.end = it->end;
				completed = true;
			}
			it = map.erase(it);
		}
		map.insert(area);
		
	}
}

bool FlashTranslator::SectorInMap(uint64_t sector_number, set<SpaceEntry>& map)
{
	auto it = map.upper_bound(SpaceEntry{ sector_number ,0 });
	if (it != map.begin()) //previous block not found
	{
		--it;
		return (it->end >= sector_number);
	}
	return false;
}

void FlashTranslator::InsertTranslatorRecord(TranslatorEntry entry)
{
	//if (translator_index >= records_in_sector) return;
	auto buf = new uint8_t[_flash->SectorSize()];	//get memory
	if(buf)
	{
		uint64_t new_translator_sector = 0;
		std::memset(buf, 0xFF, _flash->SectorSize());
		if ((translator_sector < _flash->SectorInPageCount() && (translator_page < _flash->PageCount())))	//check range
		{
				const auto entries = reinterpret_cast<TranslatorEntry *>(buf + sizeof(SectorDescriptor));	//get entries link
				entries[translator_index] = entry;
				_flash->WriteSector(translator_page, translator_sector,buf);
				translator_index++;
				if (translator_index == records_in_sector)													//if need next sector
				{
					translator_page = current_sector_descriptor.next_page;
					translator_sector = current_sector_descriptor.next_sector;
					current_sector_descriptor.next_page = 0xFFFFFFFF;
					current_sector_descriptor.next_sector = 0xFFFFFFFF;
					translator_index = 0;
					new_translator_sector = GetEmptySector();
				}
		}
		if((new_translator_sector) && (current_sector_descriptor.next_page == 0xFFFFFFFF))
		{
			//get new translator sector
			
			CutSpice(free_space, new_translator_sector);
			CutSpice(erased_space, new_translator_sector);
			auto ps = _flash->PhysicalAddres(new_translator_sector);
			//create header
			current_sector_descriptor.next_page = ps.physical_page;
			current_sector_descriptor.next_sector = ps.physical_sector;
			//write header
			memset(buf, 0xFF, _flash->SectorSize());
			*reinterpret_cast<SectorDescriptor *>(buf) = current_sector_descriptor;
			_flash->WriteSector(translator_page, translator_sector, buf);
			

		}
	}
	delete[] buf;
}
