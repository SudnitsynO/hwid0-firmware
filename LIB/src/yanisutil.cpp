#include "yanisutil.h"
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

uint64_t Stream::Write(uint8_t* buffer, uint64_t count)
{
	IoResult = READ_NOT_SUPPOTRED;
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

void Stream::Flush()
{
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

bool Stream::WriteString(char* str)
{
	if (CanWrite())
	{
		int len = strlen(str);
		if (Write(reinterpret_cast<uint8_t*>(str), len) == len)
		{
			return true;
		}
	}
	return false;
}

Event::Event(bool auto_reset)
{
	_auto_reset = auto_reset;
}

Event::Event()
{
	_auto_reset = false;
}
