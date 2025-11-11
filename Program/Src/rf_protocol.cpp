#include "rf_protocol.h"
#include "BinarySerialize.h"

RfGetId::RfGetId(const uint32_t uid): _uid(uid)
{
}

std::vector<uint8_t> RfGetId::serialize() const
{
	BinarySerializer bs;

	bs.serialise(RfCommand::GET_ID);
	bs.serialise(_uid);

	return bs.binary_data;
}

bool RfGetId::deserialize(BinaryDeSerializer& input_data)
{
	input_data.reset();
	_chance = input_data.deserialise<float>();
	return true;
}


std::vector<uint8_t> GetCurrentData::serialize() const
{
	BinarySerializer bs;
	bs.serialise(RfCommand::GET_CURRENT_DATA);
	bs.serialise(Q);
	bs.serialise(T);
	return bs.binary_data;
}

GetCurrentData::GetCurrentData(const float q, const float t)
	: Q(q), T(t)
{
}

bool GetCurrentData::deserialize(BinaryDeSerializer& input_data)
{
	return false;
}

