#pragma once
#include "DiskDrive.h"
#include "FlashDrive.h"
#include <map>
#include <list>
#include <vector>
#include <forward_list>
#include <set>
using namespace std;

class SpaceEntry
{
public:
	SpaceEntry(uint64_t begin, uint64_t end);
	bool operator<(const SpaceEntry &foo) const;
	uint64_t begin;
	uint64_t end;
};

class FlashTranslator : public DiskDrive
{
private://private types

	enum Action
	{
		ASSIGN,
		FREE,
		ERASE
	};

	struct TranslatorEntry
	{
		uint64_t logic_sector;
		PhysicalSector physical_address;
	};

	struct SectorDescriptor
	{
		uint32_t next_sector;
		uint32_t next_page;
	};


public:	//public methods
	FlashTranslator();
	void Init(FlashDrive& flash, int max_fragments);
	uint32_t SectorSize() const override;
	uint64_t SectorCount() const override;
	bool ReadSector(uint64_t sector_index, uint8_t* buffer);
	void Defrag(uint32_t last_free_page);
	uint64_t GetEmptySector();
	uint32_t FindEmptyPage();
	bool WriteSector(uint64_t sector_index, uint8_t* buffer);
	void Rebuild();
	bool EraseSector(uint64_t sector_index);
	virtual void EraseAll() override;
	bool ReadTable();
	void Test();

private:  //private methods
	void CutSpice(set<SpaceEntry>& map, uint64_t sector_number);	//remove sector from space map
	void AddSpice(set<SpaceEntry>& map, uint64_t sector_number);	//insert sector into space map
	void AddSpice(set<SpaceEntry>& map, SpaceEntry area);			//insert sectors range into  space map
	bool SectorInMap(uint64_t sector_number, set<SpaceEntry>& map);  //Test sector to free state
	void InsertTranslatorRecord(TranslatorEntry entry);

private: //private fields
	FlashDrive * _flash;
	map<uint64_t, PhysicalSector> table;
	set<SpaceEntry> free_space;	//client free space map
	set<SpaceEntry> erased_space;		//map of erased sectors
	uint64_t logic_sector_count;	//max sector count in logic space
	uint32_t records_in_sector;
	uint32_t translator_sector;	//current translator sector
	uint32_t translator_page;	    //current translator page
	uint32_t translator_index;	//next empty translator record
	SectorDescriptor current_sector_descriptor;	//sector_descriptor for current translator sector
	int _max_fragments;
};
