#include "MeasureDB.h"
#include <Buffer.h>
#include <string>

std::string MeasureHeader::to_string() const
{
	std::string result{ std::to_string(id) +" " + DateTime{ time_mark }.to_string(false) + " " + std::to_string(count_of_points) + u8"тчк."
};
	return result;
}

int MeasureHeader::index()
{
	return start_sector / record_sector_count;
}

MeasureDB::MeasureDB(DiskDrive& drive)
	:_drive(&drive)
{
}

std::vector<MeasureHeader> MeasureDB::load_headers()
{
	std::vector<MeasureHeader> headers{};
	Buffer buf{ _drive->SectorSize() };
	for (size_t i = 0; i < record_max_count; i++)
	{
		if (_drive->ReadSector(start_sector(i), buf.get()))
		{
			//прочитано успешно
			auto* header = reinterpret_cast<MeasureHeader*>(buf.get());
			if (header_is_valid(header, i))
			{
				headers.push_back(*header);
			}
			else
				break;
		}
		else
			break;
	}
	return headers;
}

int MeasureDB::points_count_in_sector() const
{
	return _drive->SectorSize() / sizeof(float);
}

MeasureData MeasureDB::read_data(int record_index)
{
	MeasureData data(nullptr);
	auto sector = start_sector(record_index);
	Buffer buf(_drive->SectorSize());
	auto* header = reinterpret_cast<MeasureHeader*>(buf.get());
	float* raw_data = reinterpret_cast<float*>(buf.get());
	int index = 0;
	if(_drive->ReadSector(sector, buf.get()))
		if (header_is_valid(header, record_index))
		{
			data.start_time = DateTime{ header->time_mark };
			data.shtyb_amount = header->shtyb;
			data.shpur_lenght = header->shpur;
			data.zaboy_id = header->zaboy_id;
			data.id = header->id;
			data.boring_pos = header->boring_pos;
			auto count_of_points = header->count_of_points;
			sector++;
			if (!_drive->ReadSector(sector, buf.get())) 
				return data;
			for (size_t i = 0; i < count_of_points; i++)
			{
				float Q = raw_data[index++];
				float T = raw_data[index++];
				data.insert_point({ T,Q });
				if (index >= points_count_in_sector())
				{
					index = 0;
					sector++;
					if (!_drive->ReadSector(sector, buf.get()))
						break;
				}
			}
		}
	return data;
}

bool MeasureDB::write_data(MeasureData& data_to_write)
{
	const auto new_record_index = load_headers().size();	//TODO: сделать бинарный поиск конца БД или хранить его в state
	auto sector = start_sector(new_record_index);
	auto end_sector = sector + record_sector_count;
	MeasureHeader header
	{
		data_to_write.start_time.total_milliseconds(),
		data_to_write.id,
		start_sector(new_record_index),
		data_to_write.shtyb_amount,
		data_to_write.shpur_lenght,
		data_to_write.zaboy_id,
		data_to_write.boring_pos,
		data_to_write.count_of_points()
	};
	
	_drive->EraseSector(start_sector(new_record_index + 1));			//стираем следующую запись
	if (!_drive->WriteSector(sector++, reinterpret_cast<uint8_t*>(&header)))	//пишем заголовок
		return false;
	Buffer buf{ _drive->SectorSize() };
	float* raw_data = reinterpret_cast<float*>(buf.get());
	int offset = 0;
	for (size_t i = 0; i < header.count_of_points; i++)
	{
		raw_data[offset++] = data_to_write.samples[i].q;
		raw_data[offset++] = data_to_write.samples[i].t;
		if(offset >= points_count_in_sector()) //если сектор заполнен 
		{
			if(!_drive->WriteSector(sector++, buf.get())) return false;
			offset = 0;
		}
	}
	if(offset)
	{
		//запишем остаток
		
		if(!_drive->WriteSector(sector++, buf.get())) return false;
	}
	//сотрем остальное
	while (sector <= end_sector)
	{
		_drive->EraseSector(sector++);
	}
	
	return true;
}

void MeasureDB::clear()
{
	_drive->EraseSector(start_sector(0));
}

void MeasureDB::format_flash()
{
	_drive->EraseAll();
}

int MeasureDB::start_sector(const int record_index) const
{
	return record_index * record_sector_count;
}

bool MeasureDB::header_is_valid(const MeasureHeader* const header, const int record_index) const
{
	if(header->start_sector != start_sector(record_index))
		return false;
	if ((header->count_of_points < 1) || (header->count_of_points > 1100))
		return false;
	if ((header->shpur < -0.1) || (header->shpur > 100))
		return false;
	if ((header->shtyb < -0.1) || (header->shtyb > 100))
		return false;
	if (header->zaboy_id > 100)	//TODO: проверять в соответствии с реальным количеством забоев
		return false;
	return true;
}