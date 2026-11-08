#include "input.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// กำหนดขา GPIO สำหรับปุ่มทั้ง 3
#define BTN_UP_PIN     GPIO_NUM_4
#define BTN_DOWN_PIN   GPIO_NUM_16
#define BTN_SELECT_PIN GPIO_NUM_17

// ตัวแปรสำหรับจำสถานะก่อนหน้า (เพื่อเช็คจังหวะกดลง) และเวลาป้องกันการลั่น
static int last_up_state = 1;
static int last_down_state = 1;
static int last_select_state = 1;
static uint32_t last_press_time = 0;

const uint32_t DEBOUNCE_TIME_MS = 200; // หน่วงเวลา 200 มิลลิวินาทีหลังกด

void input_init(void) {
    // ตั้งค่า GPIO ทั้ง 3 ขาพร้อมกัน
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE, // ไม่ใช้ Interrupt (เราจะใช้การวนลูปเช็คเอา)
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BTN_UP_PIN) | (1ULL << BTN_DOWN_PIN) | (1ULL << BTN_SELECT_PIN),
        .pull_down_en = 0,
        .pull_up_en = 1 // เปิด Internal Pull-up (สถานะปกติ = 1, กดปุ่ม = 0)
    };
    gpio_config(&io_conf);
}

button_type_t input_get_pressed_button(void) {
    int up_state = gpio_get_level(BTN_UP_PIN);
    int down_state = gpio_get_level(BTN_DOWN_PIN);
    int select_state = gpio_get_level(BTN_SELECT_PIN);
    
    button_type_t pressed_btn = BTN_NONE;
    uint32_t current_time = pdTICKS_TO_MS(xTaskGetTickCount()); // ดึงเวลาปัจจุบันของ FreeRTOS

    // เช็คว่าเลยช่วงเวลา Debounce มาแล้วหรือยัง
    if (current_time - last_press_time > DEBOUNCE_TIME_MS) {
        
        // เช็คจังหวะ Falling Edge (เพิ่งถูกกดลงเป็น 0 และสถานะก่อนหน้าคือ 1)
        if (select_state == 0 && last_select_state == 1) {
            pressed_btn = BTN_SELECT;
            last_press_time = current_time;
        } 
        else if (up_state == 0 && last_up_state == 1) {
            pressed_btn = BTN_UP;
            last_press_time = current_time;
        } 
        else if (down_state == 0 && last_down_state == 1) {
            pressed_btn = BTN_DOWN;
            last_press_time = current_time;
        }
    }

    // อัปเดตสถานะก่อนหน้าเตรียมไว้เช็ครอบต่อไป
    last_select_state = select_state;
    last_up_state = up_state;
    last_down_state = down_state;

    return pressed_btn;
}