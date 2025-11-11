#pragma once
#include "delegate.h"
#include <cstdint>
#include <memory>

/**
 * @brief Software timer interface
 *
 */
class SoftwareTimer
{
  public:

    static std::shared_ptr<SoftwareTimer> create_timer();
   
    /**
     * @brief Start this timer counter
     *
     */
    virtual bool start_periodoc(uint32_t period) = 0;
    /**
     * @brief Start this timer counter
     *
     */
    virtual bool start_one_shoot(uint32_t time) = 0;
    /**
     * @brief Stop this timer
     *
     */
    virtual void stop() = 0;

    //virtual ~SoftwareTimer() = 0;

    /**
     * @brief subscribe to this tick event
     *
     */
    Delegate<void(void)> tick;
};