#pragma once
#include <iostream>
#include <windows.h>

#include "SerialPortWin32.h"
#include "stream.h"

class SerialPortWin32 : public Stream
{
public:
	SerialPortWin32();
	void Init(const char * PortName, DWORD BaudRate, BYTE Parity, BYTE ByteSize, BYTE StopBits);
	~SerialPortWin32() override;
	bool CanRead() override;
	bool CanWrite() override;
	bool CanSeek() override;
	uint64_t Read(uint8_t* buffer, uint64_t count) override;
	uint64_t Write(const uint8_t* buffer, uint64_t count) override;
	bool Flush() override;

	void close();
	void set_read_timeout(uint32_t read_timeout);
	void set_write_timeout(uint32_t write_timeout);
private:
	void set_timeout();
private:
	HANDLE hSerial;
};

