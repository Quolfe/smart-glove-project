#include "freertos/idf_additions.h"
#include "portmacro.h"
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

void app_main(void) {
    ESP_ERROR_CHECK(init_i2c());

    display = sh1106_new();

    sh1106_config_t config = sh1106_default_config();
    ESP_ERROR_CHECK(sh1106_init(config, i2c_bus_handle, display));

    xTaskCreate(update_display, "update sh1106", 2048, NULL, 5, NULL);
}
