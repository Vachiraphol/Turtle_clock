
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    printf("ESP32 Digital Clock Starting...\n");

    while (1) {
        printf("Hello ESP32!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
