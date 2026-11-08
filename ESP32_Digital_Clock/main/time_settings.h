#ifndef TIME_SETTINGS_H
#define TIME_SETTINGS_H

#include <stdbool.h>
#include <stdint.h>

// โครงสร้างสำหรับเก็บเวลาชั่วคราวตอนอยู่ในเมนูตั้งค่า
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} DateTime_t;

// ฟังก์ชันหลัก
void time_init_ntp(const char* ssid, const char* password);
void get_current_time(DateTime_t *current);
void apply_manual_time(DateTime_t new_time);

// ฟังก์ชันตรวจสอบวันที่
bool is_leap_year(int year);
uint8_t get_days_in_month(int year, int month);
bool validate_date(int year, int month, int day);

#endif