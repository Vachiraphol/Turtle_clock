#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "time_settings.h"
#include "timezone.h"

static const char *TAG = "MAIN_TEST";

// กำหนดชื่อ WiFi และ Password ของคุณ
#define WIFI_SSID "WIFI_NAME"
#define WIFI_PASS "WIFI_PASSWORD"

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Digital Clock Starting (ESP-IDF)...");

    // ทดสอบการเชื่อมต่อ WiFi และซิงค์ NTP
    time_init_ntp(WIFI_SSID, WIFI_PASS);

    // ทดสอบ Validation ฟังก์ชัน
    ESP_LOGI(TAG, "Leap year 2024? %s", is_leap_year(2024) ? "Yes" : "No");
    ESP_LOGI(TAG, "Valid date 31 Feb 2024? %s", validate_date(2024, 2, 31) ? "Yes" : "No");

    ESP_LOGI(TAG, "Setting Timezone to Thailand (UTC+7)...");
    set_timezone(7);

    while (1) {
        DateTime_t now;
        get_current_time(&now);
        
        ESP_LOGI(TAG, "Current Time: %02d/%02d/%04d %02d:%02d:%02d", 
                 now.day, now.month, now.year, now.hour, now.minute, now.second);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}