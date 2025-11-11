#include "software_timer_FREERTOS.h"
#include "projdefs.h"

void SoftwareTimer::FreeRTOS_TimerCallback(TimerHandle_t xTimer)
{
    SoftwareTimer *pswtmr = reinterpret_cast<SoftwareTimer *>(pvTimerGetTimerID(xTimer));
    if (pswtmr->one_shoot_mode)
        xTimerStop(xTimer, pswtmr->block_time);
    if (pswtmr->one_shoot_mode)
    {
        if (!pswtmr->shoot)
        {
            pswtmr->shoot = true;
            (pswtmr->tick)();
        }
    }
    else
    {
        (pswtmr->tick)();
    }
}

bool SoftwareTimer::init(int max_block_time)
{
    block_time = max_block_time;
    timer_handle = xTimerCreate(/* Just a text name, not used by the RTOS
                                kernel. */
                                "SWTMR",
                                /* The timer period in ticks, must be
                                greater than 0. */
                                pdMS_TO_TICKS(1),
                                /* The timers will auto-reload themselves
                                when they expire. */
                                pdTRUE,
                                /* The ID is used to store a count of the
                                number of times the timer has expired, which
                                is initialised to 0. */
                                this,
                                /* Each timer calls the same callback when
                                it expires. */
                                FreeRTOS_TimerCallback);
    return timer_handle != NULL;
}

bool SoftwareTimer::start_periodoc(uint32_t period)
{
    const auto is_interrupt = xPortIsInsideInterrupt(); // define is interrupt context
    one_shoot_mode = false;
    if (is_interrupt)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerChangePeriodFromISR(timer_handle, period, &xHigherPriorityTaskWoken);
        xTimerStartFromISR(timer_handle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken != pdFALSE)
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        if (xTimerChangePeriod(timer_handle, period, block_time) == pdTRUE)
        {
            return xTimerStart(timer_handle, block_time) == pdTRUE;
        }
    }
    return false;
}

bool SoftwareTimer::start_one_shoot(uint32_t time)
{
    const auto is_interrupt = xPortIsInsideInterrupt(); // detect interrupt context
    one_shoot_mode = true;
    shoot = false;
    if (is_interrupt)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerChangePeriodFromISR(timer_handle, time, &xHigherPriorityTaskWoken);
        xTimerStartFromISR(timer_handle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken != pdFALSE)
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return true;
    }
    else
    {
        if (xTimerChangePeriod(timer_handle, time, block_time) == pdTRUE)
        {
            return xTimerStart(timer_handle, block_time) == pdTRUE;
        }
    }
    return false;
}
void SoftwareTimer::stop()
{
    const auto is_interrupt = xPortIsInsideInterrupt(); // detect interrupt context
    if (is_interrupt)
    {
         BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStopFromISR(timer_handle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken != pdFALSE)
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        xTimerStop(timer_handle, block_time);
    }
}