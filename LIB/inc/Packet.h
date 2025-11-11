#pragma once
#include <cstdint>
#include "BinarySerialize.h"


class PacketBase
{
public:

	virtual std::vector<uint8_t> serialize() const = 0;

	//virtual bool deserialize(const std::vector<uint8_t>& input_data) = 0;
	
	virtual bool deserialize(BinaryDeSerializer& input_data) = 0;
};
