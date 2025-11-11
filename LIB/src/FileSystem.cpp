#include "FileSystem.h"
#include <iostream>
#include "D:/!/CPP/Buffer.h"


namespace JNSFS
{
	constexpr uint64_t file_system_id = 0xD342B5B9B3CA;

	struct MFT_Entry
	{
		uint32_t file_id;
	};

	char const* FileSystemError::what() const
	{
		return "File system error";
	}

	FileSystem::FileSystem()
	{
	}

	void FileSystem::Init(DiskDrive& disk)
	{
		_disk = &disk;
	}

	void FileSystem::Format()
	{
		_disk->EraseAll();
		//generate bitmap file
		_fisrst_sector_descriptor.bitmap_file_size = (_disk->SectorCount() + 7) / 8;
		_fisrst_sector_descriptor.bitmap_file_sector_count = (_fisrst_sector_descriptor.bitmap_file_size + _disk->SectorSize() - 4) / _disk->SectorSize();
		_fisrst_sector_descriptor.bitmap_file_start = 1;
		for (uint64_t i = 0; i < _fisrst_sector_descriptor.bitmap_file_sector_count; i++)
		{
			Buffer buf(_disk->SectorSize());
			buf.memset(0xFF);
			_disk->WriteSector(i + _fisrst_sector_descriptor.bitmap_file_start, buf.data);
		}
		MarkSectors(0, _fisrst_sector_descriptor.bitmap_file_sector_count,true);
		_fisrst_sector_descriptor.mft_file = _fisrst_sector_descriptor.bitmap_file_sector_count + 1;
		_mft = make_shared<File>(File(*this,_fisrst_sector_descriptor.mft_file));
	}

	void FileSystem::MarkSectors(uint64_t start, uint64_t stop, bool mark_busy)
	{
		throw FileSystemError();
	}

	File::File(FileSystem& fs, uint32_t file_id): _fs(&fs), _file_id(file_id)
	{
		throw FileSystemError();
	}

	File::File(FileSystem& fs, uint64_t file_start_sector)
	{
		throw FileSystemError();
	}

	File::File()
	{
		throw FileSystemError();
	}

	File::~File()
	{
	}

	void File::Open()
	{
		throw FileSystemError();
	}

	void File::Close()
	{
		throw FileSystemError();
	}

	void File::Create()
	{
		throw FileSystemError();
	}

	bool File::CanRead()
	{
		return true;
	}

	bool File::CanWrite()
	{
		return true;
	}

	bool File::CanSeek()
	{
		return true;
	}

	uint64_t File::Read(uint8_t* buffer, uint64_t count)
	{
		throw FileSystemError();
	}

	uint64_t File::Write(uint8_t* buffer, uint64_t count)
	{
		throw FileSystemError();
	}

	uint64_t File::Position()
	{
		return _file_position;
	}

	uint64_t File::Seek(uint64_t new_position)
	{
		_file_position = new_position;
		return _file_position;
	}

	bool File::SetLength(uint64_t value)
	{
		throw FileSystemError();
	}

	uint64_t File::Length()
	{
		return _file_size;
	}

	void File::Flush()
	{
		throw FileSystemError();
	}
}
