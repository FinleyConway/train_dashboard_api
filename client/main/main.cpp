#include <esp_err.h>
#include <esp_log.h>

#include "networking/wifi/wifi.hpp"
#include "networking/wifi/provisioning/provisioning.hpp"
#include "esp_https_server.h"

#include "task_events/tcp_send_event.hpp"
#include "tasks/tcp_task.hpp"

#include "components/nfc_reader.hpp"

void connection_handle(bool has_connected) {
    if (has_connected) {
        ESP_LOGI("main", "Connected!");

        // restart the esp to take advantage of nvs
        esp_restart();
    }
    else {
        ESP_LOGI("main", "Given creds werent correct, try again!");
    }
}

extern "C" void app_main() {
    client::nfc_reader_t nfc_reader;

    ESP_ERROR_CHECK(nfc_reader.init(client::nfc_gpio_t {
        .misco = GPIO_NUM_12,
        .mosi  = GPIO_NUM_14,
        .sck   = GPIO_NUM_13,
        .cs    = GPIO_NUM_27
    }));

    ESP_LOGI("main", "Waiting for an ISO14443A Card ...");

    while (true) {
        client::nfc_tag_t tag;
        esp_err_t err = nfc_reader.read_tag(tag);

        if (err == ESP_OK) {
            ESP_LOG_BUFFER_HEXDUMP("main", tag.uid.data(), tag.uid_length, ESP_LOG_INFO);
            ESP_LOG_BUFFER_HEXDUMP("main", tag.data.data(), tag.data_length, ESP_LOG_INFO);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // static client::wifi_t wifi;

    // ESP_ERROR_CHECK(wifi.connect_from_nvs());

    // if (!wifi.wait_connection()) {
    //     client::provisioning_t wifi_prov(wifi);

    //     wifi_prov.start("esp_device", "", 1);
    //     wifi_prov.wait_connection(connection_handle);
    // }

    // ESP_LOGI("main", "connected");

    // client::tcp_send_event_t::create(10);
    // client::tcp_task_t::init(2);
}