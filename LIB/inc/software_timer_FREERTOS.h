#pragma once
//#include "FreeRTOS.h"
#include "software_timer_interface.h"
//#include "timers.h"
/*
class SoftwareTimer : public SoftwareTimerBase
{
  public:
    virtual bool init(int max_block_time);
    virtual bool start_periodoc(uint32_t period);
    virtual bool start_one_shoot(uint32_t time);
    virtual void stop();

  private:
    static void FreeRTOS_TimerCallback(TimerHandle_t xTimer);
    TimerHandle_t timer_handle;
    TickType_t block_time;
    bool one_shoot_mode = false;
    bool shoot = false;
};
*/