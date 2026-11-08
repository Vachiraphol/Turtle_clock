#include "clock.h"
#include "input.h"
#include "timezone.h"
#include "esp_log.h"

static const char *TAG = "CLOCK_STATE";

static clock_state_t current_state = STATE_NORMAL;
static DateTime_t temp_time; // ตัวแปรเก็บเวลาชั่วคราวตอนอยู่ในเมนู
static int temp_tz = 7;      // ค่าเริ่มต้น Timezone

void clock_init(void) {
    current_state = STATE_NORMAL;
    input_init(); // เรียกใช้ปุ่มกด
}

void clock_update(void) {
    button_type_t btn = input_get_pressed_button();
    if (btn == BTN_NONE) return; // ถ้าไม่มีคนกดปุ่ม ให้ออกจากฟังก์ชันทันที

    // ----------------------------------------------------
    // ลอจิกปุ่ม SELECT (สำหรับเลื่อน State / เปลี่ยนห้อง)
    // ----------------------------------------------------
    if (btn == BTN_SELECT) {
        if (current_state == STATE_NORMAL) {
            get_current_time(&temp_time); // จำเวลาปัจจุบันไว้เป็นจุดเริ่มต้น
            current_state = STATE_SET_TZ;
            ESP_LOGI(TAG, "Enter Menu -> SET_TZ");
        } 
        else if (current_state == STATE_SET_TZ) {
            current_state = STATE_SET_HOUR;
            ESP_LOGI(TAG, "Next -> SET_HOUR");
        } 
        else if (current_state == STATE_SET_HOUR) {
            current_state = STATE_SET_MIN;
            ESP_LOGI(TAG, "Next -> SET_MIN");
        } 
        else if (current_state == STATE_SET_MIN) {
            current_state = STATE_SET_SEC;
            ESP_LOGI(TAG, "Next -> SET_SEC");
        } 
        else if (current_state == STATE_SET_SEC) {
            current_state = STATE_SET_YEAR;
            ESP_LOGI(TAG, "Next -> SET_YEAR");
        } 
        else if (current_state == STATE_SET_YEAR) {
            current_state = STATE_SET_MONTH;
            ESP_LOGI(TAG, "Next -> SET_MONTH");
        } 
        else if (current_state == STATE_SET_MONTH) {
            current_state = STATE_SET_DAY;
            ESP_LOGI(TAG, "Next -> SET_DAY");
        } 
        else if (current_state == STATE_SET_DAY) {
            // ด่านสุดท้าย! ตรวจสอบความถูกต้องของวันที่ (Requirement ข้อ 4)
            if (validate_date(temp_time.year, temp_time.month, temp_time.day)) {
                apply_manual_time(temp_time); // เซฟเวลา
                set_timezone(temp_tz);        // เซฟ Timezone
                current_state = STATE_NORMAL; // กลับหน้าหลัก
                ESP_LOGI(TAG, "Save Success! Return to NORMAL");
            } else {
                ESP_LOGE(TAG, "Invalid Date! (e.g. 31 Feb) Please fix day/month.");
                // ระบบจะยังคงอยู่ที่ STATE_SET_DAY จนกว่าผู้ใช้จะแก้ให้ถูก
            }
        }
    } 
    
    // ----------------------------------------------------
    // ลอจิกปุ่ม UP / DOWN (สำหรับบวก/ลบตัวเลข)
    // ----------------------------------------------------
    else if (btn == BTN_UP || btn == BTN_DOWN) {
        int dir = (btn == BTN_UP) ? 1 : -1;

        switch (current_state) {
            case STATE_SET_TZ:
                temp_tz += dir;
                if (temp_tz > 14) temp_tz = -12;
                if (temp_tz < -12) temp_tz = 14;
                ESP_LOGI(TAG, "Adjust TZ: UTC%+d", temp_tz);
                break;
            case STATE_SET_HOUR:
                temp_time.hour = (temp_time.hour + dir + 24) % 24;
                ESP_LOGI(TAG, "Adjust Hour: %02d", temp_time.hour);
                break;
            case STATE_SET_MIN:
                temp_time.minute = (temp_time.minute + dir + 60) % 60;
                ESP_LOGI(TAG, "Adjust Min: %02d", temp_time.minute);
                break;
            case STATE_SET_SEC:
                temp_time.second = 0; // ปกติเมนูตั้งเวลา การกดปุ่มมักจะรีเซ็ตวินาทีเป็น 0
                ESP_LOGI(TAG, "Reset Sec to 00");
                break;
            case STATE_SET_YEAR:
                temp_time.year += dir;
                if (temp_time.year < 2000) temp_time.year = 2099;
                if (temp_time.year > 2099) temp_time.year = 2000;
                ESP_LOGI(TAG, "Adjust Year: %04d", temp_time.year);
                break;
            case STATE_SET_MONTH:
                temp_time.month = temp_time.month + dir;
                if (temp_time.month < 1) temp_time.month = 12;
                if (temp_time.month > 12) temp_time.month = 1;
                ESP_LOGI(TAG, "Adjust Month: %02d", temp_time.month);
                break;
            case STATE_SET_DAY:
                temp_time.day = temp_time.day + dir;
                if (temp_time.day < 1) temp_time.day = 31;
                if (temp_time.day > 31) temp_time.day = 1;
                ESP_LOGI(TAG, "Adjust Day: %02d", temp_time.day);
                break;
            default:
                break;
        }
    }
}

// Getter functions สำหรับจอ OLED ในอนาคต
clock_state_t clock_get_state(void) { return current_state; }
void clock_get_temp_time(DateTime_t *temp) { *temp = temp_time; }
int clock_get_temp_tz(void) { return temp_tz; }