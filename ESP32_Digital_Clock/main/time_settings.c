#include "time_settings.h"
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

static const char *TAG = "TIME_SETTINGS";

/* --- ส่วนจัดการ WiFi เบื้องต้นเพื่อใช้ดึง NTP --- */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying to connect to WiFi...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

static void wifi_init_sta(const char* ssid, const char* password) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = { .sta = { .threshold = { .authmode = WIFI_AUTH_WPA2_PSK } } };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
}
/* ----------------------------------------------- */


void time_init_ntp(const char* ssid, const char* password) {
    // 1. Initialize NVS (จำเป็นสำหรับ WiFi ของ ESP-IDF)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. เชื่อมต่อ WiFi
    ESP_LOGI(TAG, "Connecting to WiFi...");
    wifi_init_sta(ssid, password);
    vTaskDelay(pdMS_TO_TICKS(5000)); // รอ WiFi เชื่อมต่อ (แบบง่าย)

    // 3. ตั้งค่า SNTP
    ESP_LOGI(TAG, "Initializing SNTP...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // 4. รอจนกว่าจะซิงค์เวลาเสร็จ
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    // ตั้ง Timezone เริ่มต้น (UTC 0) Timezone จริงจะถูกตั้งในไฟล์ timezone.c
    setenv("TZ", "UTC0", 1);
    tzset();
}

void get_current_time(DateTime_t *current) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    current->year = timeinfo.tm_year + 1900;
    current->month = timeinfo.tm_mon + 1;
    current->day = timeinfo.tm_mday;
    current->hour = timeinfo.tm_hour;
    current->minute = timeinfo.tm_min;
    current->second = timeinfo.tm_sec;
}

void apply_manual_time(DateTime_t new_time) {
    struct tm t = {0};
    t.tm_year = new_time.year - 1900;
    t.tm_mon = new_time.month - 1;
    t.tm_mday = new_time.day;
    t.tm_hour = new_time.hour;
    t.tm_min = new_time.minute;
    t.tm_sec = new_time.second;
    
    time_t timeSinceEpoch = mktime(&t);
    struct timeval tv = { .tv_sec = timeSinceEpoch, .tv_usec = 0 };
    settimeofday(&tv, NULL);
}

bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

uint8_t get_days_in_month(int year, int month) {
    const uint8_t days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days[month];
}

bool validate_date(int year, int month, int day) {
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > get_days_in_month(year, month)) return false;
    return true;
}