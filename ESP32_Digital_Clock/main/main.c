#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "input.h" 
#include "time_settings.h"
#include "timezone.h"
#include "clock.h" 
#include "display.h" // <-- เพิ่ม Header หน้าจอ

static const char *TAG = "MAIN_APP";

#define WIFI_SSID "WIFI_NAME"  // <-- แก้ไขเป็นชื่อ WiFi ของคุณ
#define WIFI_PASS "WIFI_PASSWORD" // <-- แก้ไขเป็นรหัสผ่าน WiFi ของคุณ

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Digital Clock System...");

    time_init_ntp(WIFI_SSID, WIFI_PASS);
    
    // เริ่มต้นหน้าจอและ State Machine
    display_init();
    clock_init();

    while (1) {
        clock_update();
        
        // สั่งให้หน้าจออัปเดตภาพตาม State ปัจจุบันเสมอ
        display_render_menu();
        
        // ถ้าอยู่หน้าเมนูตั้งค่า ให้หน่วงเวลาน้อยลงเพื่อรับค่าปุ่มได้ทันใจ และกระพริบจอได้เนียน
        if (clock_get_state() == STATE_NORMAL) {
            vTaskDelay(pdMS_TO_TICKS(100)); // หน่วง 100ms พอดีสำหรับจอนาฬิกา
        } else {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}