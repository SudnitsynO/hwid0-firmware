#ifndef __LP_RTC_FREERTOS
#define __LP_RTC_FREERTOS

#ifdef __cplusplus

#include "DateTime.h"

extern "C" {
#endif

void LP_Init();
uint64_t LP_GetCurrentTime();
void LP_SetCurrentTime(uint64_t new_time);

#ifdef __cplusplus
}
#endif

#endif __LP_RTC_FREERTOS

