#pragma once
#include "stm32f1xx_hal.h"

#define ENTER_CRITICAL {uint32_t primask_backup = __get_PRIMASK(); __disable_irq();
#define LEAVE_CRITICAL __set_PRIMASK(primask_backup);}