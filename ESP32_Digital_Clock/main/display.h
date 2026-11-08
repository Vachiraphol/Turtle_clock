#ifndef DISPLAY_H
#define DISPLAY_H

// เริ่มต้นการทำงานของจอ OLED และ I2C
void display_init(void);

// อัปเดตหน้าจอตามสถานะของ State Machine
void display_render_menu(void);

#endif