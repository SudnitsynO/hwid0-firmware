#pragma once
#include <DateTime.h>
#include <DiskDrive.h>
#include <vector>

#include "MeasureData.h"

constexpr int record_sector_count = 50;			//число секторов для записи одного измерения
constexpr int record_max_count = 500;			//число записей всего


/*****************************************************************
 	Структура данных во флеш памяти

 	в первом секторе находится заголовок MeasureHeader
 	далее массив float [{расход, температура},{расход, температура}]
 
 *****************************************************************/

struct MeasureHeader
{
	uint64_t time_mark;	//время начала измерения
	uint32_t id;		//номер измерения
	uint32_t start_sector;//начальный сектор данных измерения
	float shtyb;		//количество штыба
	float shpur;		//длинна шпура
	uint16_t zaboy_id;	//номер забоя
	uint16_t boring_pos;//место бурения
	uint16_t count_of_points;//количество точек измерения
	std::string to_string() const;
	int index();
};

class MeasureDB
{
public:
	explicit MeasureDB(DiskDrive& drive);
	std::vector<MeasureHeader> load_headers();	//читает список всех измерений и возвратит их перечень
	MeasureData read_data(int record_index);	//читает полную информацию о записи включая данные
	bool write_data(MeasureData& data_to_write);	//пишет данные измерения в память
	void clear();								//очищает список измерений
	void format_flash();						//форматирует FLASH память
private:
	int start_sector(const int record_index) const;	//вычислит стартовый сектор записи
	bool header_is_valid(const MeasureHeader* const header, const int record_index) const; //валидация заголовка
	int points_count_in_sector() const;				//вычисляет количество точек данных в одном секторе
	DiskDrive * _drive;
};