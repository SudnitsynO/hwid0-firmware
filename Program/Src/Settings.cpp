#include "settings.h"
#include <Buffer.h>
#include "crc.h"

Settings::Settings()
	:_setup_fs(&_flash)
{
	Default();

}

void Settings::Save()
{
	//TODO: сделать сохранение списка забоев независимо от размера сектора
		size_t offset = 0;
		Buffer buf(_setup_fs.SectorSize());
		buf.memset(0xFF);
		
		uint32_t* buf_crc = (uint32_t*)buf.get();
		offset += sizeof(uint32_t);							//crc32

		int n = zaboy_list.size();
		*reinterpret_cast<int*>(buf.get() + offset) = n;	//number of elements
		offset += sizeof(n);
		for (ZaboyInfo value : zaboy_list) 
		{
			*reinterpret_cast<ZaboyInfo*>(buf.get() + offset) = value;	//zaboy names
			offset += sizeof(value);
		}

		*reinterpret_cast<float*>(buf.get() + offset) = q_scale;	//q_scale
		offset += sizeof(float);

		*reinterpret_cast<float*>(buf.get() + offset) = q_offset;	//q_offset
		offset += sizeof(float);

		*reinterpret_cast<float*>(buf.get() + offset) = t_scale;	//t_scale
		offset += sizeof(float);

		*reinterpret_cast<float*>(buf.get() + offset) = t_offset;	//t_offset
		offset += sizeof(float);

		*buf_crc = Crc32Block(CRC32_INIT_VALUE, buf.get() + sizeof(uint32_t), buf.size() - sizeof(uint32_t));

		_setup_fs.WriteSector(0x33, buf.get());
}

void Settings::Load()
{
	size_t offset = 0;
	Buffer buf(_setup_fs.SectorSize());
	if (_setup_fs.ReadSector(0x33, buf.get()))
	{
		auto local_crc = Crc32Block(CRC32_INIT_VALUE, buf.get() + sizeof(uint32_t), buf.size() - sizeof(uint32_t));
		auto buf_crc = *reinterpret_cast<uint32_t*>(buf.get() + offset); offset += sizeof(uint32_t);	//crc
		if (local_crc == buf_crc)
		{
			zaboy_list.clear();
			auto n = *reinterpret_cast<int*>(buf.get() + offset); offset += sizeof(int);	//number of elements
			for (int i = 0; i < n; i++)
			{
				auto name = *reinterpret_cast<ZaboyInfo*>(buf.get() + offset); offset += sizeof(ZaboyInfo);	//number of elements
				zaboy_list.push_back(name);
			}

			q_scale = *reinterpret_cast<float*>(buf.get() + offset); offset += sizeof(float);	//q_scale
			q_offset = *reinterpret_cast<float*>(buf.get() + offset); offset += sizeof(float);	//q_offset
			t_scale = *reinterpret_cast<float*>(buf.get() + offset); offset += sizeof(float);	//t_scale
			t_offset = *reinterpret_cast<float*>(buf.get() + offset); offset += sizeof(float);	//t_offset
		}
		else
		{
			Default();
		}
	}
	else
		Default();
}

void Settings::Default()
{
	ZaboyInfo zaboy_info;
	zaboy_list.clear();
	for (size_t i = 1; i < 5; i++)
	{
		sprintf(zaboy_info.name, u8"Забой %i", i);
		zaboy_list.push_back(zaboy_info);
	}
	//q_scale = q_offset = t_scale = t_offset = .5;
	q_offset = 0.207;
	q_scale = 0.112;
	t_scale = 0.164;
	t_offset = 0.140;
}

void Settings::Init()
{
	//_setup_fs.Init(_drive, 100);
	Buffer buf(_flash.SectorSize());
	_flash.ReadSector(0, 0, buf.get());
	if (*reinterpret_cast<unsigned long*>(buf.get()) == 0xFFFFFFFF)
		_setup_fs.init_table();
	_setup_fs.load();
}

void Settings::format()
{
	_setup_fs.init_table();
	//Save();
}
