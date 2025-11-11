#include "stream.h"
#include <string.h>

Stream::Stream()
{
	ReadTimeout = 0;
	WriteTimeout = 0;
	IoResult = OK;
}

Stream::~Stream()
{
}

bool Stream::CanRead()
{
	return false;
}

bool Stream::CanWrite()
{
	return false;
}

bool Stream::CanSeek()
{
	return false;
}

uint64_t Stream::Read(uint8_t* buffer, uint64_t count)
{
	IoResult = READ_NOT_SUPPOTRED;
	return 0;
}

uint64_t Stream::Write(const uint8_t* buffer, uint64_t count)
{
	IoResult = WRITE_NOT_SUPPOTRED;
	return 0;
}

uint64_t Stream::Position()
{
	IoResult = SEEK_NOT_SUPPORTED;
	return 0;
}

uint64_t Stream::Seek(uint64_t new_position)
{
	IoResult = SEEK_NOT_SUPPORTED;
	return 0;
}

bool Stream::SetLength(uint64_t value)
{
	IoResult = SEEK_NOT_SUPPORTED;
	return false;
}

uint64_t Stream::Length()
{
	IoResult = SEEK_NOT_SUPPORTED;
	return 0;
}

bool Stream::Flush()
{
	return true;
}

int Stream::ReadByte(void)
{
	if (CanRead())
	{
		uint8_t ch;
		if (Read(&ch, 1) == 1)
			return ch;
		else
			return -1;
	}
	return -1;
}

bool Stream::WriteByte(uint8_t byte)
{
	if (CanWrite())
	{
		if (Write(&byte, 1) == 1)
			return true;
		else
			return false;
	}
	return false;
}

bool Stream::WriteString(const char* str)
{
	if (CanWrite())
	{
		auto len = strlen(str);
		if (Write(reinterpret_cast<const uint8_t*>(str), len) == len)
		{
			return true;
		}
	}
	return false;
}

bool Stream::WriteVector(const std::vector<uint8_t> data)
{
	if (CanWrite())
	{
		auto len = data.size();
		if (Write(data.data(), len) == len)
		{
			return true;
		}
	}
	return false;
}
