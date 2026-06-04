#include <esp_err.h>
#include <cJSON.h>

#include "networking/wifi/wifi.hpp"
#include "networking/wifi//wifi_provisioning.hpp"
#include "esp_https_server.h"

#include "task_events/tcp_send_event.hpp"
#include "tasks/tcp_task.hpp"

// https://developer.espressif.com/blog/2025/06/basic_http_server/

char g_ssid[32];
char g_password[63];
EventGroupHandle_t g_event_group = nullptr;
static constexpr EventBits_t g_received_bit = BIT0;
static constexpr EventBits_t g_fail_bit = BIT1;

httpd_handle_t start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = nullptr;

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI("HTTP", "Http server created");

        return server;
    }

    ESP_LOGE("HTTP", "Failled to create http server");

    return nullptr;
}

void stop_webserver(httpd_handle_t server) {
    if (server != nullptr) {
        httpd_stop(server);
    }
}

static esp_err_t wifi_cred_post_handler(httpd_req_t* req) {
    ESP_LOGI("HTTP", "Receiving wifi creds from http");

    ESP_LOGI("HTTP", "Buffer size: %lu", req->content_len + 1);

    char* buffer = new char[req->content_len + 1];
    size_t offset = 0;
    int bytes_recv = 0;

    if (buffer == nullptr) {
        ESP_LOGE("HTTP", "Failed to allocate memory of %d bytes", req->content_len + 1);
        httpd_resp_send_500(req);
        xEventGroupSetBits(g_event_group, g_fail_bit);

        return ESP_FAIL;
    }

    while (offset < req->content_len) {
        bytes_recv = httpd_req_recv(req, buffer + offset, req->content_len - offset);

        if (bytes_recv <= 0) {
            if (bytes_recv == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }

            delete[] buffer;
            xEventGroupSetBits(g_event_group, g_fail_bit);

            return ESP_FAIL;
        }

        offset += bytes_recv;
    }
    buffer[offset] = '\0';

    cJSON* root = cJSON_Parse(buffer);

    if (root == nullptr) {
        ESP_LOGE("HTTP", "Invalid JSON");
        delete[] buffer;
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        xEventGroupSetBits(g_event_group, g_fail_bit);

        return ESP_FAIL;
    }

    cJSON* ssid = cJSON_GetObjectItem(root, "ssid");
    cJSON* password = cJSON_GetObjectItem(root, "password");

    if (!cJSON_IsString(ssid) || !cJSON_IsString(password)) {
        ESP_LOGE("HTTP", "Missing fields");

        cJSON_Delete(root);
        delete[] buffer;

        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing fields");
        xEventGroupSetBits(g_event_group, g_fail_bit);

        return ESP_FAIL;
    }

    char* ssid_str = ssid->valuestring;
    size_t ssid_len = strlen(ssid_str);

    char* password_str = password->valuestring;
    size_t password_len = strlen(password_str);

    if (ssid_len > 32) {
        ESP_LOGE("HTTP", "SSID too long");

        cJSON_Delete(root);
        delete[] buffer;

        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "SSID too long");
        xEventGroupSetBits(g_event_group, g_fail_bit);

        return ESP_FAIL;
    }

    if (password_len > 63) {
        ESP_LOGE("HTTP", "Password too long");

        cJSON_Delete(root);
        delete[] buffer;

        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Password too long");
        xEventGroupSetBits(g_event_group, g_fail_bit);

        return ESP_FAIL;
    }

    ESP_LOGI("HTTP", "Got creds: %s, %s", ssid_str, password_str);

    std::strncpy(g_ssid, ssid_str, ssid_len);
    std::strncpy(g_password, password_str, password_len);
    xEventGroupSetBits(g_event_group, g_received_bit);

    cJSON_Delete(root);
    delete[] buffer;

    httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}

// TODO: Need to figure out how to test wifi station without turning off AP until its succesfull

extern "C" void app_main() {
    static client::wifi_t wifi;

    ESP_ERROR_CHECK(wifi.connect());

    if (!wifi.wait_connection()) {
        client::wifi_provisioning_t wifi_prov(wifi);

        ESP_ERROR_CHECK(wifi_prov.start_softap("esp", "", 1));

        g_event_group = xEventGroupCreate();
        xEventGroupClearBits(g_event_group, g_received_bit | g_fail_bit);
        httpd_handle_t server = start_webserver();

        static const httpd_uri_t wifi_cred_uri = {
            .uri       = "/wifi_cred",               
            .method    = HTTP_POST,        
            .handler   = wifi_cred_post_handler, 
            .user_ctx  = NULL            
        };

        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &wifi_cred_uri));

        bool try_connection = true;
        while (try_connection) {
            EventBits_t bits = xEventGroupWaitBits(
                g_event_group,
                g_received_bit | g_fail_bit,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY
            );

            if (bits & g_received_bit) {
                ESP_ERROR_CHECK(wifi_prov.start_sta_provisioning(g_ssid, g_password));

                if (wifi_prov.wait_connection()) {
                    ESP_ERROR_CHECK(wifi_prov.stop());
                    stop_webserver(server);

                    try_connection = false;

                    ESP_LOGW("HTTP", "Success!");

                    esp_restart();
                }
                else {
                    xEventGroupClearBits(g_event_group, g_received_bit | g_fail_bit);

                    ESP_LOGW("HTTP", "Failed to connect, retrying...");
                }
            }
            else {
                xEventGroupClearBits(g_event_group, g_received_bit | g_fail_bit);

                ESP_LOGW("HTTP", "Failed to ssid and password, retrying...");
            }
        }
    }

    ESP_LOGI("YEYY", "WORKS??");
}