#pragma once
#include "stream.h"
#include "DiskDrive.h"
#include <exception>
#include <memory>

using namespace std;

namespace JNSFS
{
	struct FisrstSectorDescriptor
	{
		uint64_t file_system_id;
		uint64_t volume_id;
		uint64_t mft_file;
		uint64_t bitmap_file_start;
		uint64_t bitmap_file_size;
		uint64_t bitmap_file_sector_count;
		uint32_t mft_records_in_sector;
		//uint32_t bitmap_records_in_sector;
	};

	class FileSystemError : public std::exception {
	public:
		char const* what() const override;
	};

	class FileError : public std::exception {
	public:
		char const* what() const override;
	};

	class ErrorFileExist : public FileSystemError {
	public:
		char const* what() const override;
	};

	class ErrorFileNotFound : public FileSystemError {
	public:
		char const* what() const override;
	};

	class ErrorNoSpice : public FileSystemError {
	public:
		char const* what() const override;
	};

	class FileSystem
	{
		friend class File;
	public:
		FileSystem();
		void Init(DiskDrive & disk);
		uint64_t TotalSize();
		uint64_t FreeSpice();
		void Format();
	private:
		void MarkSectors(uint64_t start, uint64_t stop, bool mark_busy);	//Mark sectors is busy 
		DiskDrive * _disk;
		shared_ptr<File> _mft;
		FisrstSectorDescriptor _fisrst_sector_descriptor;
	};

	class File : public Stream
	{
		friend class FileSystem;
	public:
		File(FileSystem &fs, uint32_t file_id);
		File(FileSystem &fs, uint64_t file_start_sector);
		File();
		~File();
		virtual void Open();
		virtual void Close();
		virtual void Create();
		virtual bool CanRead();
		virtual bool CanWrite();
		virtual bool CanSeek();
		virtual uint64_t Read(uint8_t* buffer, uint64_t count);
		virtual uint64_t Write(uint8_t* buffer, uint64_t count);
		virtual uint64_t Position();
		virtual uint64_t Seek(uint64_t new_position);
		virtual bool SetLength(uint64_t value);
		virtual uint64_t Length();
		virtual void Flush();
	private:
		FileSystem * _fs;
		uint32_t _file_id;
		uint64_t _file_size;
		uint64_t _file_position;
		uint64_t _first_sector;
	};

	

	
};

