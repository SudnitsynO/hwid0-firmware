#pragma once
#include "Packet.h"
#include <BinarySerialize.h>
#include <cstdint>

/************************************************
 *      Формат пакета воздух нисходящий
 *      
 *  -----------------------------------------
 *  !  параметр   !  тип      ! пояснение	!
 *  -----------------------------------------
 *  !  payloadCRC ! uint32    !             !
 *  !  command    ! uint8     !				!
 *  !  payload    ! xxx       !             !
 *  -----------------------------------------
 *  при расчете CRC за начальное значение берется
 *  ID адресата или значение 0xFFFFFFFF для всех   
 *  
 *  Для восходящего пакета формат тот же, но ID 
 *  получателя = ID отправителя
 *
 */

enum class RfCommand : uint8_t
{
	UNKNOWN = 0,
	PING = 1,					//проверка наличия связи
	GET_RESULT = 2,				//запускает чтение результата измерения
	GET_CURRENT_DATA = 3,		//запускает трансляцию измеряемых значений на 3 секунды
	GET_ID = 4,					//запрос идентификатора / поиск устройств
	SETUP = 5,					//запрос настроек прибора
	GET_RESULT_LIST = 6,		//запускает чтение списка результатов измерений
	GET_FRAGMENT = 7			//запрос фрагмента
};

class RfGetId : public PacketBase
{
public:
	RfGetId(const uint32_t uid);
	virtual std::vector<uint8_t> serialize() const;
	virtual bool deserialize(BinaryDeSerializer& input_data);
	float _chance;
private:
	uint32_t _uid;
};

class GetCurrentData : public PacketBase
{
public:
	std::vector<uint8_t> serialize() const override;

	GetCurrentData(const float q, const float t);

	virtual bool deserialize(BinaryDeSerializer& input_data);
	float Q;
	float T;
};