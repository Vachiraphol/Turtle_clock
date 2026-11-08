#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "input.h" 
#include "time_settings.h"
#include "timezone.h"
// เดี๋ยวเราจะมาเพิ่ม #include "clock.h" ตรงนี้ในสเต็ปถัดไป

static const char *TAG = "MAIN_TEST";

// เก็บค่า WiFi ไว้ก่อน เพราะเดี๋ยวก็ต้องใช้ซิงค์เวลา
#define WIFI_SSID "WIFI_NAME"
#define WIFI_PASS "WIFI_PASSWORD"

void app_main(void)
{
    ESP_LOGI(TAG, "System Ready for State Machine...");
    
    // เตรียมพื้นที่ว่างไว้เรียกใช้ clock_init(); และ clock_update();
    
    while (1) {
        // ลูปหลักเตรียมรอ State Machine
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}