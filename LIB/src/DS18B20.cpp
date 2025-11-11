#include "DS18B20.h"

vector<DS18B20> DS18B20::devices;

DS18B20::DS18B20(OneWireBase& bus, const uint64_t id) 
{
	id_ = id;
	bus_ = &bus;
}

void DS18B20::discovery_all(OneWireBase& bus)
{
	devices.clear();
	uint64_t id = 0;
	bus.reset_search();
	while (bus.search_next(id))
	{
		if((id & 0xFF) == 0x28)
			devices.emplace_back(bus, id);
	}
}

bool DS18B20::start_conversion_all() const
{
	if(bus_->select_all_devices())
	{
		bus_->send_byte(0x44);
		return true;
	}
	return false;
}

bool DS18B20::start_conversion() const
{
	if (id_ == 0)
	{
		if (!bus_->select_all_devices())
			return false;
	}
	else if (!bus_->select_device(id_))
		return false;
	bus_->send_byte(0x44);
	return true;
}

bool DS18B20::get_temperature(float& temperature) const
{
	if (id_ == 0)
	{
		if (!bus_->select_all_devices())
			return false;
	}
	else if (!bus_->select_device(id_))
			return false;
	bus_->send_byte(0xBE);
	uint8_t buf[8];
	uint8_t local_crc = 0;
	for (auto& i : buf)
	{
		i = bus_->read_byte();
		local_crc = OneWireBase::do_crc8(local_crc, i);
	}
	const auto crc = bus_->read_byte();
	if (crc != local_crc)
		return false;
	const int16_t temp = buf[0] | (buf[1] << 8);
	temperature = temp / 16.0;
	return true;
}

void DS18B20::set_resolution(DS18B20_Resolution res) const
{
	if (id_ == 0)
	{
		if (!bus_->select_all_devices())
			return;
	}
	else if (!bus_->select_device(id_))
		return;
	bus_->send_byte(0x4E);
	bus_->send_byte(0x0);
	bus_->send_byte(0x0);
	bus_->send_byte(static_cast<uint8_t>(res) << 5);

}

uint64_t DS18B20::get_id() const
{
	return  id_;
}
