#include "onewirebase.h"
#include <cstdint>
#include <cstddef>

/* From AN187 MaximIntegrated */
static unsigned char ds_crc_table[] = {
 0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
 157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220,
 35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98,
 190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
 70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7,
 219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154,
 101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36,
 248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185,
 140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
 17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
 175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
 50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
 202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139,
 87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
 233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
 116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53 };



void OneWireBase::send_byte(uint8_t byte)
{
	for (size_t i = 0; i < 8; i++)
	{
		send_bit(byte & (1 << i));
	}
}

uint8_t OneWireBase::read_byte()
{
	uint8_t byte = 0;
	for (size_t i = 0; i < 8; i++)
	{
		byte = (byte >> 1) | (read_bit() ? 0x80 : 0);
	}
	return byte;
}

void OneWireBase::send_cmd(OneWireCommands cmd)
{
	send_byte(static_cast<uint8_t>(cmd));
}

void OneWireBase::reset_search()
{
	last_discrepancy_ = 0;
	last_device_flag_ = false;
	last_family_discrepancy_ = 0;
}

bool OneWireBase::search_next(uint64_t& rom_code)
{
	bool search_direction;
	// initialize for search
	uint8_t id_bit_number = 1;
	uint8_t last_zero = 0;
	uint64_t rom_byte_mask = 1;
	bool search_result = false;
	uint8_t crc8 = 0;
	// if the last call was not the last one
	if (!last_device_flag_)
	{
		// 1-Wire reset
		if (!reset())
		{
			// reset the search
			last_discrepancy_ = 0;
			last_device_flag_ = false;
			last_family_discrepancy_ = 0;
			return false;
		}
		// issue the search command
		send_cmd(OneWireCommands::search_rom);
		// loop to do the search
		for (size_t i = 0; i < 64; i++)
		{
			// read a bit and its complement
			const bool id_bit = read_bit();
			const bool cmp_id_bit = read_bit();
			// check for no devices on 1-wire
			if (id_bit && cmp_id_bit)
				break;
			else
			{
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit)
					search_direction = id_bit; // bit write value for search
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < last_discrepancy_)
						search_direction = ((ROM_NO & rom_byte_mask));
					else
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == last_discrepancy_);
					// if 0 was picked then record its position in LastZero
					if (search_direction == 0)
					{
						last_zero = id_bit_number;
						// check for Last discrepancy in family
						if (last_zero < 9)
							last_family_discrepancy_ = last_zero;
					}
				}
				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1)
					ROM_NO |= rom_byte_mask;	//set
				else
					ROM_NO &= ~rom_byte_mask;	//reset
				// serial number search direction write bit
				send_bit(search_direction);
				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;
			}
		}
		//CRC calc
		uint64_t temp_rom = ROM_NO;
		for(int i = 0; i < 8; i++)
		{
		   crc8 = do_crc8(crc8, temp_rom & 0xFF);
		   temp_rom >>= 8;
		}
		// if the search was successful then
		if (!((id_bit_number < 65) || (crc8 != 0)))
		{
			// search successful so set	LastDiscrepancy, LastDeviceFlag, search_result
				last_discrepancy_ = last_zero;
			// check for last device
			if (last_discrepancy_ == 0)
				last_device_flag_ = true;

			search_result = true;
		}
		if (!search_result || !(ROM_NO & 0xFF))
		{
			last_discrepancy_ = 0;
			last_device_flag_ = false;
			last_family_discrepancy_ = 0;
			search_result = false;
		}
	}
	if (search_result)
		rom_code = ROM_NO;
	return search_result;
}

bool OneWireBase::select_device(uint64_t rom_code)
{
	if(reset())
	{
		send_cmd(OneWireCommands::mach_rom);
		for (size_t i = 0; i < 8; i++)
		{
			send_byte(rom_code & 0xFF);
			rom_code >>= 8;
		}
		return true;
	}
	return false;
}

bool OneWireBase::select_all_devices()
{
	if (reset())
	{
		send_cmd(OneWireCommands::skip_rom);
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current
// global 'crc8' value.
// Returns current global crc8 value
//
uint8_t OneWireBase::do_crc8(uint8_t crc, uint8_t value)
{
	// See Application Note 27
	return ds_crc_table[crc ^ value];
}


