#pragma once
#include "onewirebase.h"
#include <vector>

enum class DS18B20_Resolution
{
	RES_9_BIT = 0,		//94ms
	RES_10_BIT = 0,		//188ms
	RES_11_BIT = 0,		//375ms
	RES_12_BIT = 0		//750ms
};

using namespace  std;
class DS18B20
{
public:
	DS18B20(OneWireBase& bus, uint64_t id);
	static void discovery_all(OneWireBase& bus);	//discovery all DS18B20 devices
	static vector<DS18B20> devices;					//list of discovered devices

	bool start_conversion_all() const;				//start conversion on all sensors
	bool start_conversion() const;					//start conversion on current sensors
	bool get_temperature(float &temperature) const; //read temperature value on current sensor
	void set_resolution(DS18B20_Resolution res) const;
	uint64_t get_id() const;

private:
	OneWireBase * bus_;
	uint64_t id_;
};
