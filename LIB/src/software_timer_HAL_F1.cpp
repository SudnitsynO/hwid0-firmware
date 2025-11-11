#include "software_timer_HAL.h"
#include "cortex_m3_critical.h"
#include <algorithm>
#include <list>
#include <memory>
#include <stdint.h>

std::shared_ptr<SoftwareTimer> SoftwareTimer::create_timer()
{
    auto new_timer = std::make_shared<SoftwareTimerHAL>();
    new_timer->_weak_this = new_timer;
    return new_timer;
}

std::list<std::shared_ptr<SoftwareTimerHAL>> SoftwareTimerHAL::active_list{};
uint32_t SoftwareTimerHAL::global_time{};

void SoftwareTimerHAL::OneMsTimerIsr()
{
    global_time++;
    std::list<std::shared_ptr<SoftwareTimerHAL>>::iterator it{};
    ENTER_CRITICAL
    {
        it = active_list.begin();
    }
    LEAVE_CRITICAL;
    while (it != active_list.end())
    {
        auto &timer = *it;
        if (timer)
        {
            if (timer->_active)
            {
                const int32_t delta = timer->_tick_time - global_time;
                if (delta <= 0)
                {
                    //время вышло
                    timer->tick();
                    if (timer->_period)
                    {
                        timer->_tick_time += timer->_period;
                    }
                    else
                    {
                        timer->stop();
                    }
                }
                ENTER_CRITICAL
                {
                    it++;
                }
                LEAVE_CRITICAL;
            }
            else
            {
                ENTER_CRITICAL
                {
                    it = active_list.erase(it);
                }
                LEAVE_CRITICAL;
            }
        }
        else
        {
            ENTER_CRITICAL
            {
                it = active_list.erase(it);
            }
            LEAVE_CRITICAL;
        }
    }
}

bool SoftwareTimerHAL::start_periodoc(uint32_t period)
{
    if (period == 0)
    {
        stop();
        return false;
    }
    _period = period;
    _tick_time = global_time + _period;
    if (!_active)
    {
        _active = true;

        ENTER_CRITICAL
        {
            active_list.emplace_back(_weak_this.lock());
        }
        LEAVE_CRITICAL;
    }
    return true;
}

bool SoftwareTimerHAL::start_one_shoot(uint32_t time)
{
    if (time == 0)
    {
        stop();
        return false;
    }
    _tick_time = global_time + time;
    _period = 0;
    if (!_active)
    {
        _active = true;
        ENTER_CRITICAL
        {
            active_list.emplace_back(_weak_this.lock());
        }
        LEAVE_CRITICAL;
    }
    return true;
}

void SoftwareTimerHAL::stop()
{
    _active = false;
}

SoftwareTimerHAL::~SoftwareTimerHAL()
{
    
}

/*

SoftwareTimer *SoftwareTimer::first_active = nullptr;

static uint32_t global_time = 0;

uint32_t SoftwareTimer::_global_next_tick_time = 0;

void SoftwareTimer::TimerCallback()
{
    // 1 ms ISR
    global_time++;
    int32_t min_delta = 0;
    SoftwareTimer *prev = nullptr;
    SoftwareTimer *next = nullptr;
    ENTER_CRITICAL
    {
        SoftwareTimer *p = first_active;
        while (p)
        {
            if (p->active)
            {
                const int32_t delta = p->_tick_time - global_time;
                if (delta <= 0)
                {
                    const auto old_first = first_active;
                    p->tick();
                    if (first_active != old_first)  //если обработчик таймера
                        if (prev == nullptr)        //активировал другой таймер
                            prev = first_active;    //пометим его как предыдущий
                    if (p->_one_shoot_mode)
                    {
                        p->active = false;
                    }
                    else
                    {
                        p->_tick_time += p->_period;
                    }
                }
                int32_t new_delta = p->_tick_time - global_time;
                min_delta = std::min(new_delta, min_delta);
            }
            if (!p->active)
            {
                //удаляем неактивный p
                if (prev) //если есть предыдущий
                {
                    prev->next_active = p->next_active; //сшиваем
                    next = p->next_active;
                    //
                }
                else
                {
                    //предыдущего нет, значит удаляем первый
                    first_active = p->next_active; //сдвигаем первый элемент
                    next = p->next_active;
                }
            }
            else
            {
                next = p->next_active;
                prev = p;
            }
            p = next;
        }
    }
    LEAVE_CRITICAL;
    _global_next_tick_time = min_delta + global_time;
}

bool SoftwareTimer::init(int max_block_time)
{
    return true;
}

bool SoftwareTimer::start_periodoc(uint32_t period)
{
    ENTER_CRITICAL
    {
        _period = period;
        _one_shoot_mode = false;
        _tick_time = period + global_time;
        activate_timer();
    }
    LEAVE_CRITICAL;
    return true;
}

bool SoftwareTimer::start_one_shoot(uint32_t time)
{
    ENTER_CRITICAL
    {
        _period = 0;
        _one_shoot_mode = true;
        _tick_time = time + global_time;
        activate_timer();
    }
    LEAVE_CRITICAL;
    return true;
}

void SoftwareTimer::stop()
{
    active = false;
}

void SoftwareTimer::activate_timer()
{
    ENTER_CRITICAL
    {
        if (!active)
        {
            next_active = first_active;
            first_active = this;
            active = true;
        }
    }
    LEAVE_CRITICAL;
}

SoftwareTimer::~SoftwareTimer()
{
    ENTER_CRITICAL
    {
        SoftwareTimer *p = first_active;
        SoftwareTimer *prev = nullptr;
        while (p)
        {
            if (p == this)
            {
                if (prev)
                {
                    prev->next_active = p->next_active;
                }
                else
                {
                    first_active = p->next_active;
                }
                p = nullptr;
            }
            else
            {
                prev = p;
                p = p->next_active;
            }
        }
    }
    LEAVE_CRITICAL;
}

*/
