#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "input.h" // <-- ดึงไลบรารีปุ่มที่เราเขียนขึ้นมา

static const char *TAG = "MAIN_TEST";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Input Button Test...");
    
    // 1. เรียกใช้งานฟังก์ชันเริ่มต้นปุ่มกด
    input_init();
    ESP_LOGI(TAG, "Buttons Initialized. Press UP, DOWN, or SELECT...");

    // 2. ลูปตรวจสอบการกดปุ่ม (ทำงานทุก 50ms)
    while (1) {
        button_type_t btn = input_get_pressed_button();
        
        if (btn == BTN_UP) {
            ESP_LOGI(TAG, "Button PRESSED: [UP]");
        } else if (btn == BTN_DOWN) {
            ESP_LOGI(TAG, "Button PRESSED: [DOWN]");
        } else if (btn == BTN_SELECT) {
            ESP_LOGI(TAG, "Button PRESSED: [SELECT]");
        }
        
        // หน่วงเวลาสั้นๆ เพื่อไม่ให้ลูปกินซีพียู 100%
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}