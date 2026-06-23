#pragma once

#include <array>

#include <esp_log.h>
#include <driver/gpio.h>

#include <pn532.h>
#include <pn532_driver_spi.h>

// https://docs.nxp.com/bundle/NTAG213_215_216/page/topics/memory_organization.html

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
        // (NTAG2XX_NTAG216 * 4 bytes) - read_only bytes
        static constexpr size_t max_nfc_user_memory = 888;

        std::array<uint8_t, 7> uid{};
        uint8_t uid_length = 0;

        std::array<uint8_t, max_nfc_user_memory> data{};
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
                ESP_LOGW(c_tag, "failed to initialize PN532");

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
        int16_t get_user_page_end() {
            // TODO: i could cache this to speed up reading process???
            NTAG2XX_MODEL ntag_model = NTAG2XX_UNKNOWN;
            ESP_ERROR_CHECK_WITHOUT_ABORT(ntag2xx_get_model(&m_pn532_io, &ntag_model));

            // REF: https://docs.nxp.com/bundle/NTAG213_215_216/page/topics/memory_organization.html
            switch (ntag_model) {
                case NTAG2XX_NTAG213: return 39;
                case NTAG2XX_NTAG215: return 129;
                case NTAG2XX_NTAG216: return 225;
                default: return -1;
            }
        }

        esp_err_t read_page(nfc_tag_t& tag) {
            constexpr uint8_t page_size = 4;
            int16_t end_page = get_user_page_end();

            if (end_page < 0) {
                ESP_LOGW(c_tag, "Can't read the unknown tag!");

                return ESP_FAIL;
            }

            // read the user memory 
            int16_t bytes_read = 0;

            for (int16_t page = c_user_start_page; page <= end_page; page += 4) {
                uint8_t buf[16];

                esp_err_t err = ntag2xx_read_page(&m_pn532_io, page, buf, 16);
                if (err != ESP_OK) {
                    ESP_LOGW(c_tag, "A partical read was captured!"); // maybe return a enum state? full read, partical read, fail?
                    break;
                }

                size_t offset = (page - c_user_start_page) * page_size;

                // copy the read pages
                std::memcpy(tag.data.data() + offset, buf, 16);

                // keep track of pages read
                bytes_read += 16;
            }

            tag.data_length = bytes_read;

            return ESP_OK;
        }

    private:
        pn532_io_t m_pn532_io;

        static constexpr const char* c_tag = "nfc_reader";
        static constexpr spi_host_device_t c_spi_host_nfc = SPI3_HOST;
        static constexpr uint32_t c_spi_clockrate = 4000000;
        static constexpr uint8_t c_user_start_page = 4;
    };
}