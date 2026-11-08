#ifndef TIMEZONE_H
#define TIMEZONE_H

// ฟังก์ชันสำหรับตั้งค่าเขตเวลา (รับค่าเป็นชั่วโมง เช่น 7 สำหรับ UTC+7)
void set_timezone(int utc_offset_hours);

#endif