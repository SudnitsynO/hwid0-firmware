#include "HAL_UID.h"
#include <crc.h>
#include "stm32f1xx_hal.h"

uint32_t HAL_UID::get_uid()
{
	uint32_t UID[3];
	UID[0] = HAL_GetUIDw0();
	UID[1] = HAL_GetUIDw1();
	UID[2] = HAL_GetUIDw2();
	return Crc32Block(CRC32_INIT_VALUE, UID, sizeof(UID));
}
