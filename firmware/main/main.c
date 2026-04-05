#include "freertos/idf_additions.h"
#include "sh1106.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/i2c_master.h>

#define SCL_GPIO_NUM 22
#define SDA_GPIO_NUM 23

i2c_master_bus_handle_t i2c_bus_handle;
SemaphoreHandle_t i2c_mutex;

sh1106_t *display;

esp_err_t init_i2c(void) {
    esp_err_t err;
    i2c_master_bus_config_t i2c_master_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = SCL_GPIO_NUM,
        .sda_io_num = SDA_GPIO_NUM,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    err = i2c_new_master_bus(&i2c_master_config, &i2c_bus_handle);
    if (err != ESP_OK)
        return err;

    i2c_mutex = xSemaphoreCreateMutex();

    return ESP_OK;
}

void update_display(void *arg) {
    while (1) {
        sh1106_update_display(display, i2c_mutex);
        vTaskDelay(33 / portTICK_PERIOD_MS);
    }
}

void draw_status(void *arg) {
}

void draw_finger_status(void *arg) {
    uint8_t gap = 8;
    uint8_t bits_1[] = {
        0, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 2, 2, 2, 2, 2, 2, 2, 1,
        1, 2, 2, 2, 3, 2, 2, 2, 1,
        1, 2, 2, 3, 3, 2, 2, 2, 1,
        1, 2, 2, 2, 3, 2, 2, 2, 1,
        1, 2, 2, 2, 3, 2, 2, 2, 1,
        1, 2, 2, 2, 3, 2, 2, 2, 1,
        1, 2, 2, 3, 3, 3, 2, 2, 1,
        1, 2, 2, 2, 2, 2, 2, 2, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    uint8_t bits_2[] = {
        0, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 2, 2, 2, 2, 2, 2, 2, 1,
        1, 2, 2, 2, 3, 2, 2, 2, 1,
        1, 2, 2, 3, 2, 3, 2, 2, 1,
        1, 2, 2, 2, 2, 3, 2, 2, 1,
        1, 2, 2, 2, 3, 2, 2, 2, 1,
        1, 2, 2, 3, 2, 2, 2, 2, 1,
        1, 2, 2, 3, 3, 3, 2, 2, 1,
        1, 2, 2, 2, 2, 2, 2, 2, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    uint8_t bits_3[] = {
        0, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 2, 2, 2, 2, 2, 2, 2, 1,
        1, 2, 2, 3, 3, 2, 2, 2, 1,
        1, 2, 2, 2, 2, 3, 2, 2, 1,
        1, 2, 2, 2, 3, 2, 2, 2, 1,
        1, 2, 2, 2, 2, 3, 2, 2, 1,
        1, 2, 2, 2, 2, 3, 2, 2, 1,
        1, 2, 2, 3, 3, 2, 2, 2, 1,
        1, 2, 2, 2, 2, 2, 2, 2, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    uint8_t bits_4[] = {
        0, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 2, 2, 2, 2, 2, 2, 2, 1,
        1, 2, 2, 3, 2, 3, 2, 2, 1,
        1, 2, 2, 3, 2, 3, 2, 2, 1,
        1, 2, 2, 3, 3, 3, 2, 2, 1,
        1, 2, 2, 2, 2, 3, 2, 2, 1,
        1, 2, 2, 2, 2, 3, 2, 2, 1,
        1, 2, 2, 2, 2, 3, 2, 2, 1,
        1, 2, 2, 2, 2, 2, 2, 2, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    bitmap_t bmap_1 = {
        .data = bits_1,
        .x_size = 9,
        .y_size = 10,
        .x = 128 / 2 - bmap_1.x_size * 2 - gap * 3 / 2,
        .y = 64 - bmap_1.y_size,
    };
    bitmap_t bmap_2 = {
        .data = bits_2,
        .x_size = 9,
        .y_size = 10,
        .x = 128 / 2 - bmap_2.x_size - gap / 2,
        .y = 64 - bmap_2.y_size,
    };
    bitmap_t bmap_3 = {
        .data = bits_3,
        .x_size = 9,
        .y_size = 10,
        .x = 128 / 2 + gap / 2,
        .y = 64 - bmap_3.y_size,
    };
    bitmap_t bmap_4 = {
        .data = bits_4,
        .x_size = 9,
        .y_size = 10,
        .x = 128 / 2 + bmap_4.x_size + gap * 3 / 2,
        .y = 64 - bmap_4.y_size,
    };
    bitmap_t bmaps[] = { bmap_1, bmap_2, bmap_3, bmap_4 };
    bool on = false;
    while (1) {
        for (int i = 0; i < sizeof(bmaps) / sizeof(*bmaps); i++) {
            bitmap_t bmap = bmaps[i];
            for (uint8_t x = 0; x < bmap.x_size; x++) {
                for (uint8_t y = 0; y < bmap.y_size; y++) {
                    uint8_t data_at = bmap.data[y * bmap.x_size + x];
                    bool set_pixel = (data_at == 1) || (data_at == 2 && on) || (data_at == 3 && !on);
                    sh1106_draw_pixel(display, x + bmap.x, y + bmap.y, set_pixel);
                }
            }
        }
        on ^= true;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(init_i2c());

    display = sh1106_new();

    sh1106_config_t config = sh1106_default_config();
    ESP_ERROR_CHECK(sh1106_init(config, i2c_bus_handle, display));

    xTaskCreate(update_display, "update sh1106 display", 2048, NULL, 5, NULL);
    xTaskCreate(draw_finger_status, "draw finger status", 2048, NULL, 6, NULL);
}
