#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

// กำหนดชื่อเรียกปุ่มต่างๆ เพื่อง่ายต่อการนำไปใช้ใน State Machine
typedef enum {
    BTN_NONE = 0,
    BTN_SELECT,
    BTN_UP,
    BTN_DOWN
} button_type_t;

// ฟังก์ชันหลัก
void input_init(void);
button_type_t input_get_pressed_button(void);

#endif