#include "display.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "clock.h"
#include "time_settings.h"
#include <string.h>
#include <stdio.h>

#define I2C_MASTER_NUM    I2C_NUM_0
#define OLED_I2C_ADDR     0x3C
#define OLED_WIDTH        128
#define OLED_HEIGHT       64

static uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];

// ชุดตัวอักษรขนาด 5x7 (เฉพาะตัวที่ใช้บ่อย เพื่อประหยัดหน่วยความจำ)
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 0: Space
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 1: '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 2: '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 3: '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 4: '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 5: '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 6: '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 7: '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 8: '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 9: '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 10: '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // 11: ':'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // 12: '/'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // 13: '+'
    {0x08, 0x08, 0x08, 0x08, 0x08}, // 14: '-'
    {0x00, 0x41, 0x22, 0x14, 0x08}, // 15: '>'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 16: 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 17: 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 18: 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 19: 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 20: 'E'
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 21: 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 22: 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 23: 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 24: 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 25: 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 26: 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 27: 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 28: 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 29: 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 30: 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 31: 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 32: 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 33: 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 34: 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 35: 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 36: 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 37: 'V'
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 38: 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 39: 'X'
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 40: 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}  // 41: 'Z'
};

// ฟังก์ชันสำหรับส่งคำสั่ง (Command) ไปยัง OLED
static void oled_cmd(uint8_t cmd) {
    uint8_t data[2] = {0x00, cmd};
    i2c_master_write_to_device(I2C_MASTER_NUM, OLED_I2C_ADDR, data, 2, 1000 / portTICK_PERIOD_MS);
}

void display_init(void) {
    // 1. ตั้งค่า I2C (SDA=21, SCL=22)
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .scl_io_num = 22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    // 2. ลำดับคำสั่งเปิดจอ SSD1306 แบบมาตรฐาน
    uint8_t init_cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12, 
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    for (int i = 0; i < sizeof(init_cmds); i++) oled_cmd(init_cmds[i]);
}

static void draw_pixel(int x, int y, int color) {
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;
    if (color) oled_buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));
    else       oled_buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
}

static void draw_char(int x, int y, char c, int scale) {
    int idx = 0; // Default to Space
    if (c >= 'a' && c <= 'z') c -= 32; // แปลงพิมพ์เล็กเป็นพิมพ์ใหญ่
    if (c >= '0' && c <= '9') idx = c - '0' + 1;
    else if (c == ':') idx = 11;
    else if (c == '/') idx = 12;
    else if (c == '+') idx = 13;
    else if (c == '-') idx = 14;
    else if (c == '>') idx = 15;
    else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 16;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 7; j++) {
            if (font5x7[idx][i] & (1 << j)) {
                for (int sx = 0; sx < scale; sx++)
                    for (int sy = 0; sy < scale; sy++)
                        draw_pixel(x + i * scale + sx, y + j * scale + sy, 1);
            }
        }
    }
}

static void draw_string(int x, int y, const char *str, int scale) {
    while (*str) {
        draw_char(x, y, *str++, scale);
        x += 6 * scale; // เว้นช่องไฟตัวอักษร
    }
}

static void display_update(void) {
    for (int page = 0; page < 8; page++) {
        oled_cmd(0xB0 + page);
        oled_cmd(0x00);
        oled_cmd(0x10);
        
        uint8_t data[129];
        data[0] = 0x40; // Control byte สำหรับส่ง Data
        memcpy(&data[1], &oled_buffer[page * OLED_WIDTH], OLED_WIDTH);
        i2c_master_write_to_device(I2C_MASTER_NUM, OLED_I2C_ADDR, data, 129, 1000 / portTICK_PERIOD_MS);
    }
}

void display_render_menu(void) {
    memset(oled_buffer, 0, sizeof(oled_buffer)); // ล้างหน้าจอ
    clock_state_t state = clock_get_state();
    char buf[32];

    if (state == STATE_NORMAL) {
        DateTime_t now;
        get_current_time(&now);
        // แสดงเวลาตัวใหญ่
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.hour, now.minute, now.second);
        draw_string(15, 15, buf, 2); 
        // แสดงวันที่ตัวเล็ก
        snprintf(buf, sizeof(buf), "UTC%+d %02d/%02d/%04d", clock_get_temp_tz(), now.day, now.month, now.year);
        draw_string(30, 45, buf, 1);
    } 
    else {
        DateTime_t t;
        clock_get_temp_time(&t);
        draw_string(20, 0, "SETTINGS MENU", 1);
        
        // ลอจิกทำตัวกระพริบ (Blink ทุกๆ 500ms)
        bool blink = (xTaskGetTickCount() * portTICK_PERIOD_MS / 500) % 2;

        if (state == STATE_SET_TZ) {
            draw_string(0, 20, "TIMEZONE", 1);
            if (blink) {
                snprintf(buf, sizeof(buf), "> UTC%+d", clock_get_temp_tz());
                draw_string(0, 35, buf, 2);
            }
        } else if (state == STATE_SET_HOUR) {
            draw_string(0, 20, "SET HOUR", 1);
            if (blink) { snprintf(buf, sizeof(buf), "> %02d", t.hour); draw_string(0, 35, buf, 2); }
        } else if (state == STATE_SET_MIN) {
            draw_string(0, 20, "SET MINUTE", 1);
            if (blink) { snprintf(buf, sizeof(buf), "> %02d", t.minute); draw_string(0, 35, buf, 2); }
        } else if (state == STATE_SET_SEC) {
            draw_string(0, 20, "SET SECOND", 1);
            if (blink) { draw_string(0, 35, "> 00", 2); }
        } else if (state == STATE_SET_YEAR) {
            draw_string(0, 20, "SET YEAR", 1);
            if (blink) { snprintf(buf, sizeof(buf), "> %04d", t.year); draw_string(0, 35, buf, 2); }
        } else if (state == STATE_SET_MONTH) {
            draw_string(0, 20, "SET MONTH", 1);
            if (blink) { snprintf(buf, sizeof(buf), "> %02d", t.month); draw_string(0, 35, buf, 2); }
        } else if (state == STATE_SET_DAY) {
            draw_string(0, 20, "SET DAY", 1);
            if (blink) { snprintf(buf, sizeof(buf), "> %02d", t.day); draw_string(0, 35, buf, 2); }
        }
    }
    display_update(); // ส่งข้อมูลขึ้นจอ
}