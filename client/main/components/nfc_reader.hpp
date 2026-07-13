#pragma once

#include <driver/gpio.h>

#include <pn532.h>
#include <pn532_driver_spi.h>

// https://docs.nxp.com/bundle/NTAG213_215_216/page/topics/memory_organization.html

namespace client {
    class nfc_tag_t;

    struct nfc_gpio_t {
        gpio_num_t miso = GPIO_NUM_NC;
        gpio_num_t mosi  = GPIO_NUM_NC;
        gpio_num_t sck   = GPIO_NUM_NC;
        gpio_num_t cs    = GPIO_NUM_NC;

        gpio_num_t reset = GPIO_NUM_NC;
        gpio_num_t irq   = GPIO_NUM_NC;
    };

    enum class nfc_read_state_t {
        fail,
        partical_read,
        full_read
    };

    class nfc_reader_t {
    public:
        nfc_reader_t() = default;
        ~nfc_reader_t();

        nfc_reader_t(const nfc_reader_t&) = delete;
        nfc_reader_t& operator=(const nfc_reader_t&) = delete;

        nfc_reader_t(nfc_reader_t&&) noexcept = default;
        nfc_reader_t& operator=(nfc_reader_t&&) noexcept = default;

        esp_err_t init(const nfc_gpio_t& gpio);

        nfc_read_state_t read_tag(nfc_tag_t& tag, int32_t timeout = 0);

    private:
        int16_t get_user_page_end(nfc_tag_t& tag);

        nfc_read_state_t read_page(nfc_tag_t& tag);

    private:
        pn532_io_t m_pn532_io;

        static constexpr const char* c_tag = "nfc_reader";
        static constexpr spi_host_device_t c_spi_host_nfc = SPI3_HOST;
        static constexpr uint32_t c_spi_clockrate = 4000000;
        static constexpr uint8_t c_user_start_page = 4;
    };
}