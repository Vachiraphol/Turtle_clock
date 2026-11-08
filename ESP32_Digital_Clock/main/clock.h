#ifndef CLOCK_H
#define CLOCK_H

#include "time_settings.h"

// กำหนด "ห้อง" หรือ "สถานะ" ของหน้าจอ
typedef enum {
    STATE_NORMAL,
    STATE_SET_TZ,
    STATE_SET_HOUR,
    STATE_SET_MIN,
    STATE_SET_SEC,
    STATE_SET_YEAR,
    STATE_SET_MONTH,
    STATE_SET_DAY
} clock_state_t;

void clock_init(void);
void clock_update(void);

// ฟังก์ชันสำหรับส่งข้อมูลไปให้หน้าจอ OLED วาดในอนาคต
clock_state_t clock_get_state(void);
void clock_get_temp_time(DateTime_t *temp);
int clock_get_temp_tz(void);

#endif