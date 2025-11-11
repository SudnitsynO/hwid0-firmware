#pragma once
#include <cstdint>


enum class OneWireCommands
{
	search_rom	= 0xF0,
	read_rom	= 0x33,
	mach_rom	= 0x55,
	skip_rom	= 0xCC,
	alarm_search = 0xEC
};

class OneWireBase
{
public:
	//platform specific implement
	virtual bool reset() = 0;
	virtual void send_bit(bool bit) = 0;
	virtual bool read_bit() = 0;
	//user functions
	void send_byte(uint8_t byte);
	uint8_t read_byte();
	void send_cmd(OneWireCommands cmd);
	void reset_search();
	bool search_next(uint64_t & rom_code);
	bool select_device(uint64_t rom_code);
	bool select_all_devices();
	static uint8_t do_crc8(uint8_t crc, uint8_t value);
private:
	bool last_device_flag_ = false;
	uint8_t last_discrepancy_ = 0;
	uint8_t last_family_discrepancy_ = 0;
	uint64_t ROM_NO = 0;
};