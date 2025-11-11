#pragma once
#include "software_timer_interface.h"
#include <list>
#include <stdint.h>

class SoftwareTimerHAL : public SoftwareTimer
{
  friend SoftwareTimer;
  public:
    virtual bool start_periodoc(uint32_t period);
    /**
     * @brief Start this timer counter
     *
     */
    virtual bool start_one_shoot(uint32_t time);
    /**
     * @brief Stop this timer
     *
     */
    virtual void stop();

    virtual ~SoftwareTimerHAL();

    static void OneMsTimerIsr();

  private:
    static std::list<std::shared_ptr<SoftwareTimerHAL>> active_list;
    static uint32_t global_time;
    uint32_t _period{};
    uint32_t _tick_time{};
    bool _active{false};
    std::weak_ptr<SoftwareTimerHAL> _weak_this;
};

/*
class SoftwareTimer : public SoftwareTimerBase
{
  public:
    virtual bool init(int max_block_time = 0);
    virtual bool start_periodoc(uint32_t period);
    virtual bool start_one_shoot(uint32_t time);
    virtual void stop();
    static void TimerCallback();
    virtual ~SoftwareTimer();

  private:
    void activate_timer();
    // static std::vector<SoftwareTimer*> active_timers;
    // int block_time;

    uint32_t _period;
    uint32_t _tick_time;
    static uint32_t _global_next_tick_time;
    bool _one_shoot_mode = false;
    bool active = false;

    static SoftwareTimer *first_active;
    SoftwareTimer *next_active;
};
*/