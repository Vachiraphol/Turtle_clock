#include "timezone.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "esp_log.h"

static const char *TAG = "TIMEZONE";

void set_timezone(int utc_offset_hours) {
    char tz_str[32];
    
    // POSIX TZ Format: ทิศทางเครื่องหมายจะกลับด้านกับที่เราคุ้นเคย
    // เช่น ไทย (UTC+7) ต้องเขียนเป็น UTC-7
    // ดังนั้นเราจึงต้องใส่เครื่องหมายลบ (-) หน้า utc_offset_hours
    snprintf(tz_str, sizeof(tz_str), "UTC%+d", -utc_offset_hours);
    
    // ตั้งค่า Environment Variable "TZ" ของระบบ
    setenv("TZ", tz_str, 1);
    tzset(); // สั่งให้อัปเดตเวลาตาม TZ ใหม่
    
    ESP_LOGI(TAG, "Timezone set to UTC%+d (%s)", utc_offset_hours, tz_str);
}