#include "components/nfc_reader.hpp"

#include <array>
#include <cstring>

#include <esp_log.h>

#include "components/nfc_tag.hpp"

namespace client {
    nfc_reader_t::~nfc_reader_t() {
        pn532_release(&m_pn532_io);
    }

    esp_err_t nfc_reader_t::init(const nfc_gpio_t& gpio) {
        ESP_ERROR_CHECK(pn532_new_driver_spi(
            gpio.miso,
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
            ESP_LOGW(c_tag, "Failed to initialize PN532");
            pn532_release(&m_pn532_io);

            return err;
        }

        ESP_LOGI(c_tag, "PN532 initialised!");

        return err;
    }  

    nfc_read_state nfc_reader_t::read_tag(nfc_tag_t& tag, int32_t timeout) {
        nfc_tag_t::uid_t uid{};
        uint8_t uid_length = 0;

        esp_err_t err = pn532_read_passive_target_id(
            &m_pn532_io,
            PN532_BRTY_ISO14443A_106KBPS,
            uid.data(),
            &uid_length,
            timeout
        );

        if (err != ESP_OK) {
            ESP_LOGW(c_tag, "Failed to read tag!");
            
            return nfc_read_state::fail;
        }
        
        tag.set_uid(std::move(uid), uid_length);

        return read_page(tag);
    }

    int16_t nfc_reader_t::get_user_page_end(nfc_tag_t& tag) {
        NTAG2XX_MODEL ntag_model = tag.get_model();

        if (ntag_model == NTAG2XX_UNKNOWN) {
            ntag2xx_get_model(&m_pn532_io, &ntag_model);
            tag.set_model(ntag_model);
        }

        switch (ntag_model) {
            case NTAG2XX_NTAG213: return 39;
            case NTAG2XX_NTAG215: return 129;
            case NTAG2XX_NTAG216: return 225;
            default: return -1;
        }
    }

    nfc_read_state nfc_reader_t::read_page(nfc_tag_t& tag) {
        constexpr uint8_t page_size = 4;
        const int16_t end_page = get_user_page_end(tag);

        if (end_page < 0) {
            ESP_LOGW(c_tag, "Can't read the unknown tag!");

            return nfc_read_state::fail;
        }

        // read the user memory 
        nfc_read_state state = nfc_read_state::full_read;
        nfc_tag_t::data_t data;
        int16_t bytes_read = 0;

        // read 4 pages (16 bytes) at a time
        for (int16_t page = c_user_start_page; page <= end_page; page += 4) {
            uint8_t buf[16];

            // read 4 pages
            esp_err_t err = ntag2xx_read_page(&m_pn532_io, page, buf, 16);
            if (err != ESP_OK) {
                ESP_LOGW(c_tag, "A partical read was captured!"); 
                state = nfc_read_state::partical_read;
                
                break;
            }

            // copy the read pages
            const size_t offset = (page - c_user_start_page) * page_size;
            std::memcpy(data.data() + offset, buf, 16);

            // keep track of pages read
            bytes_read += 16;
        }

        tag.set_payload(std::move(data), bytes_read);

        return state;
    }
}