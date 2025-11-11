#include "init_freertos.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "semphr.h"

static SemaphoreHandle_t init_semaphore = NULL;

void init_stop()
{
    while(1);
}

void init_init()
{
    init_semaphore = xSemaphoreCreateBinary();
    if(init_semaphore == NULL) init_stop();
}

void init_task_exit()
{
    xSemaphoreGive(init_semaphore);
    vTaskDelete(NULL);
}

void wait_init()
{
    xSemaphoreTake(init_semaphore, portMAX_DELAY);
    xSemaphoreGive(init_semaphore);
}
