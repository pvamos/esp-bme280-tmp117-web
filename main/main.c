#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c.h"
#include "bme280.h"
#include "tmp117.h"
#include "wifi_sta.h"
#include "httpd.h"

static const char *TAG = "MAIN";

void app_main() {
    ESP_LOGI(TAG, "Starting application...");

    // Connect to Wi-Fi
    if (wifi_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi");
        return;
    }
    ESP_LOGI(TAG, "Connected to Wi-Fi successfully");

    // Initialize I2C bus
    i2c_master_bus_handle_t i2c_bus_handle;
    if (i2c_master_init(&i2c_bus_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus");
        return;
    }
    ESP_LOGI(TAG, "I2C bus initialized successfully");

    // Initialize BME280 sensor
    if (bme280_init(i2c_bus_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BME280 sensor");
        return;
    }
    ESP_LOGI(TAG, "BME280 initialized successfully");

    // Initialize TMP117 sensor
    if (tmp117_init(i2c_bus_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize TMP117 sensor");
        return;
    }
    ESP_LOGI(TAG, "TMP117 initialized successfully");

    // Start HTTP server
    if (http_server_start() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return;
    }
    ESP_LOGI(TAG, "HTTP server started successfully");

    ESP_LOGI(TAG, "Initialization complete. Waiting for web requests...");
}
