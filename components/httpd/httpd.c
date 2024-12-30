#include "httpd.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "bme280.h"
#include "tmp117.h"

static const char *TAG = "HTTPD";

// HTTP GET handler for sensor data
static esp_err_t sensor_data_handler(httpd_req_t *req) {
    int32_t raw_temp_bme280, raw_pressure_bme280, raw_humidity_bme280;
    int16_t raw_temp_tmp117;
    float compensated_temp_bme280, compensated_pressure_bme280, compensated_humidity_bme280;
    float compensated_temp_tmp117;

    // Read raw data from BME280
    if (bme280_read_raw(&raw_temp_bme280, &raw_pressure_bme280, &raw_humidity_bme280) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read raw data from BME280");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Calculate compensated data for BME280
    if (bme280_calculate_compensated(
            raw_temp_bme280, raw_pressure_bme280, raw_humidity_bme280,
            &compensated_temp_bme280, &compensated_pressure_bme280, &compensated_humidity_bme280) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to calculate compensated data for BME280");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Read raw data from TMP117
    if (tmp117_read_raw(&raw_temp_tmp117) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read raw data from TMP117");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Calculate compensated data for TMP117
    if (tmp117_calculate_compensated(raw_temp_tmp117, &compensated_temp_tmp117) != ESP_OK) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Generate HTML response
    char response[512];
    snprintf(response, sizeof(response),
             "<html><body>"
             "<h1>Sensor Data</h1>"
             "<h2>BME280</h2>"
             "<p>Raw Temperature: %" PRId32 "</p>"
             "<p>Raw Pressure: %" PRId32 "</p>"
             "<p>Raw Humidity: %" PRId32 "</p>"
             "<p>Compensated Temperature: %.2f&deg;C</p>"
             "<p>Compensated Pressure: %.4f hPa</p>"
             "<p>Compensated Humidity: %.4f%%</p>"
             "<h2>TMP117</h2>"
             "<p>Raw Temperature: %" PRId16 "</p>"
             "<p>Compensated Temperature: %.4f&deg;C</p>"
             "</body></html>",
             raw_temp_bme280, raw_pressure_bme280, raw_humidity_bme280,
             compensated_temp_bme280, compensated_pressure_bme280, compensated_humidity_bme280,
             raw_temp_tmp117, compensated_temp_tmp117);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

// HTTP request handler for invalid routes (does nothing)
static esp_err_t default_handler(httpd_req_t *req) {
    // Just close the connection without sending a response

    return ESP_OK;
}

// Function to start the HTTP server
esp_err_t http_server_start() {
    httpd_handle_t server = NULL;
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.max_open_sockets = 3;           // Set concurrent connections
    config.lru_purge_enable = true;         // Free least recently used connections
    config.recv_wait_timeout = 5;           // Set socket receive timeout
    config.send_wait_timeout = 5;           // Set socket send timeout
    config.stack_size = 16384;               // Set stack size
    config.uri_match_fn = httpd_uri_match_wildcard; // Set a default handler for unregistered routes

    httpd_uri_t sensor_data_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = sensor_data_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t catch_all_uri = {
        .uri = "/*", // Matches all other routes
        .method = HTTP_GET,
        .handler = default_handler,
        .user_ctx = NULL,
    };

    ESP_ERROR_CHECK(httpd_start(&server, &config));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &sensor_data_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &catch_all_uri));
    ESP_LOGI(TAG, "HTTP server started successfully");

    return ESP_OK;
}