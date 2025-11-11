#include "mutex_freertos.h"
#include "task.h"

Mutex::Mutex()
{
    _mutex_handle = xSemaphoreCreateBinary();
    xSemaphoreGive(_mutex_handle);
}

void Mutex::lock()
{
    xSemaphoreTake(_mutex_handle, portMAX_DELAY);
}

void Mutex::lock_from_isr()
{
    BaseType_t xTaskWokenByReceive = pdFALSE;
    xSemaphoreTakeFromISR(_mutex_handle, &xTaskWokenByReceive);
    if (xTaskWokenByReceive != pdFALSE)
    {
        /* We should switch context so the ISR returns to a different task.
        NOTE:  How this is done depends on the port you are using.  Check
        the documentation and examples for your port. */
        taskYIELD();
    }
}

bool Mutex::lock(int timeout)
{
    return xSemaphoreTake(_mutex_handle, timeout) == pdTRUE;
}

void Mutex::release()
{

    xSemaphoreGive(_mutex_handle);
}

void Mutex::release_from_isr()
{
    BaseType_t xTaskWokenByReceive = pdFALSE;
    xSemaphoreGiveFromISR(_mutex_handle, &xTaskWokenByReceive);
    if (xTaskWokenByReceive != pdFALSE)
    {
        /* We should switch context so the ISR returns to a different task.
        NOTE:  How this is done depends on the port you are using.  Check
        the documentation and examples for your port. */
        taskYIELD();
    }
}

Mutex::~Mutex()
{
    vSemaphoreDelete(_mutex_handle);
}
