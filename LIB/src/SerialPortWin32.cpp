#include "SerialPortWin32.h"

SerialPortWin32::SerialPortWin32()
{
    hSerial = INVALID_HANDLE_VALUE;
    ReadTimeout = 0;
    WriteTimeout = 0;
}

void SerialPortWin32::Init(const char* PortName, DWORD BaudRate, BYTE Parity, BYTE ByteSize, BYTE StopBits)
{
    LPCTSTR sPortName = L"COM5";
    hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, 0);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            std::cout << "serial port does not exist.\n";
        }
        std::cout << "some other error occurred.\n";
        return;
    }

    DCB dcbSerialParams = { };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        std::cout << "getting state error\n";
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return;
    }
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        std::cout << "error setting serial port state\n";
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    	return;
    }
    set_timeout();
}

SerialPortWin32::~SerialPortWin32()
{
    CloseHandle(hSerial);
}

bool SerialPortWin32::CanRead()
{
    return hSerial != INVALID_HANDLE_VALUE;
}

bool SerialPortWin32::CanWrite()
{
    return hSerial != INVALID_HANDLE_VALUE;
}

bool SerialPortWin32::CanSeek()
{
	return false;
}

uint64_t SerialPortWin32::Read(uint8_t* buffer, uint64_t count)
{
	
    if (CanRead())
    {
        DWORD NumberOfBytesRead = 0;
        ReadFile(hSerial,buffer,count,&NumberOfBytesRead,nullptr);
        return NumberOfBytesRead;
    }
    return 0;
}

uint64_t SerialPortWin32::Write(const uint8_t* buffer, uint64_t count)
{
	if(CanWrite())
	{
        DWORD NumberOfBytesWritten = 0;
        WriteFile(hSerial, buffer, count, &NumberOfBytesWritten, nullptr);
        return NumberOfBytesWritten;
	}
    return 0;
}

void SerialPortWin32::close()
{
    CloseHandle(hSerial);
    hSerial = INVALID_HANDLE_VALUE;
}

void SerialPortWin32::set_read_timeout(uint32_t read_timeout)
{
    ReadTimeout = read_timeout;
    set_timeout();
}

void SerialPortWin32::set_write_timeout(uint32_t write_timeout)
{
    WriteTimeout = write_timeout;
    set_timeout();
}

void SerialPortWin32::set_timeout()
{
    if (CanRead())
    {
        COMMTIMEOUTS CommTimeouts;
        CommTimeouts.ReadIntervalTimeout = ReadTimeout;
        CommTimeouts.ReadTotalTimeoutConstant = ReadTimeout;
        CommTimeouts.WriteTotalTimeoutConstant = WriteTimeout;
        CommTimeouts.ReadTotalTimeoutMultiplier = 0;
        CommTimeouts.WriteTotalTimeoutMultiplier = 0;
        SetCommTimeouts(hSerial, &CommTimeouts);
    }
}
