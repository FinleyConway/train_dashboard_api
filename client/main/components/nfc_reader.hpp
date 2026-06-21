#pragma once

#include <array>

#include <esp_log.h>
#include <driver/gpio.h>

#include <pn532.h>
#include <pn532_driver_spi.h>

namespace client {
    struct nfc_gpio_t {
        gpio_num_t misco = GPIO_NUM_NC;
        gpio_num_t mosi  = GPIO_NUM_NC;
        gpio_num_t sck   = GPIO_NUM_NC;
        gpio_num_t cs    = GPIO_NUM_NC;

        gpio_num_t reset = GPIO_NUM_NC;
        gpio_num_t irq   = GPIO_NUM_NC;
    };

    struct nfc_tag_t {
        std::array<uint8_t, 7> uid{};
        uint8_t uid_length = 0;

        std::array<uint16_t, 231 * 4> data{};
        size_t data_length = 0;
    };

    class nfc_reader_t {
    public:
        nfc_reader_t() = default;
        
        ~nfc_reader_t() {
            pn532_release(&m_pn532_io);
        }

        esp_err_t init(const nfc_gpio_t& gpio) {
            ESP_ERROR_CHECK(pn532_new_driver_spi(
                gpio.misco,
                gpio.mosi,
                gpio.sck, 
                gpio.cs, 
                gpio.reset,
                gpio.irq,
                c_spi_host_nfc,
                c_spi_clockrate,
                &m_pn532_io
            ));
            
            esp_err_t err = pn532_init(&m_pn532_io);
            if (err != ESP_OK) {
                ESP_LOGW("main", "failed to initialize PN532");

                pn532_release(&m_pn532_io);
            }

            ESP_LOGI(c_tag, "nfc reader initialised!");

            return err;
        }   

        esp_err_t read_tag(nfc_tag_t& tag, int32_t timeout = 0) {
            esp_err_t err = pn532_read_passive_target_id(
                &m_pn532_io,
                PN532_BRTY_ISO14443A_106KBPS,
                tag.uid.data(),
                &tag.uid_length,
                timeout
            );

            if (err != ESP_OK) {
                ESP_LOGW(c_tag, "Failed to read tag!");
                
                return err;
            }

            return read_page(tag);
        }

    private:
        int16_t get_page_size() {
            constexpr int16_t unknown_page = -1;
            NTAG2XX_MODEL ntag_model = NTAG2XX_UNKNOWN;
            esp_err_t err = ntag2xx_get_model(&m_pn532_io, &ntag_model);

            if (err != ESP_OK) {
                return unknown_page;
            }

            int16_t page_size = unknown_page;

            switch (ntag_model) {
                case NTAG2XX_NTAG213:
                    page_size = 45;
                    ESP_LOGI(c_tag, "found NTAG213 target (or maybe NTAG203)");
                    break;

                case NTAG2XX_NTAG215:
                    page_size = 135;
                    ESP_LOGI(c_tag, "found NTAG215 target");
                    break;

                case NTAG2XX_NTAG216:
                    page_size = 231;
                    ESP_LOGI(c_tag, "found NTAG216 target");
                    break;

                default:
                    ESP_LOGI(c_tag, "Found unknown NTAG target!");
                    break;
            }

            return page_size;
        }

        esp_err_t read_page(nfc_tag_t& tag) {
            esp_err_t err = ESP_FAIL;
            int16_t page_size = get_page_size();

            if (page_size < 0) {
                ESP_LOGW(c_tag, "Can't read the unknown tag!");

                return err;
            }

            tag.data_length = page_size * 4;

            for (int16_t page = 0; page < page_size; page += 4) {
                uint8_t buf[16];
                err = ntag2xx_read_page(&m_pn532_io, page, buf, 16);
                
                if (err != ESP_OK) {
                    ESP_LOGI(c_tag, "Failed to read page %d", page);

                    return err;
                }

                std::memcpy(
                    tag.data.data() + (page * 4),
                    buf,
                    sizeof(buf)
                );
            }

            return err;
        }

    private:
        pn532_io_t m_pn532_io;

        static constexpr const char* c_tag = "nfc";
        static constexpr spi_host_device_t c_spi_host_nfc = SPI3_HOST;
        static constexpr uint32_t c_spi_clockrate = 1000000;
    };
}