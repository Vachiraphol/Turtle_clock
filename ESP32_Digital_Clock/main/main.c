#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "input.h" 
#include "time_settings.h"
#include "timezone.h"
#include "clock.h" // <-- เพิ่ม Header ของ State Machine

static const char *TAG = "MAIN_APP";

// ใส่ชื่อและรหัสผ่าน WiFi / Hotspot ของคุณ
#define WIFI_SSID "WIFI_NAME"
#define WIFI_PASS "WIFI_PASSWORD"

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Digital Clock starting...");

    // 1. เชื่อมต่อ WiFi และซิงค์เวลามาตรฐาน (UTC+0)
    time_init_ntp(WIFI_SSID, WIFI_PASS);
    
    // 2. เริ่มต้น State Machine (ฟังก์ชันนี้จะเรียก input_init() ให้อัตโนมัติ)
    clock_init();

    while (1) {
        // อัปเดตสถานะปุ่มและ State Machine ตลอดเวลา
        clock_update();
        
        // ถ้าอยู่หน้าปัดนาฬิกาปกติ ให้แสดงเวลาปัจจุบันทุกๆ 1 วินาที
        if (clock_get_state() == STATE_NORMAL) {
            DateTime_t now;
            get_current_time(&now);
            ESP_LOGI(TAG, "NORMAL: %02d/%02d/%04d %02d:%02d:%02d (TZ: UTC%+d)", 
                     now.day, now.month, now.year, now.hour, now.minute, now.second, clock_get_temp_tz());
            vTaskDelay(pdMS_TO_TICKS(1000)); 
        } else {
            // ถ้าอยู่ในโหมดตั้งค่า ให้รันลูปเร็วขึ้นเพื่อให้ปุ่มตอบสนองทันใจ
            // (ไม่ต้องกลัวปุ่มลั่น เพราะ Debounce ใน input.c จัดการให้แล้ว)
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}