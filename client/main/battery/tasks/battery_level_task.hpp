#pragma once

#include <freertos/FreeRTOS.h>
#include <esp_adc/adc_oneshot.h>

// https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/peripherals/adc.html

namespace client {
    struct system_bus_t;

    class battery_level_task_t {
    public:
        explicit battery_level_task_t(system_bus_t& bus);
        ~battery_level_task_t();

        battery_level_task_t(const battery_level_task_t&) = delete;
        battery_level_task_t& operator=(const battery_level_task_t&) = delete;

        battery_level_task_t(battery_level_task_t&&) noexcept = delete;
        battery_level_task_t& operator=(battery_level_task_t&&) noexcept = delete;
    
    public:
        void init();

    private:
        static void run_wrapper(void* paramaters);
    
        void run();

        static constexpr int battery_to_adc_voltage(float voltage);

        float get_percentage(int voltage) const;

        void setup_adc();

        int get_adc_millivolts();

    private:
        system_bus_t& m_bus;

        TaskHandle_t m_handle = nullptr;
        adc_oneshot_unit_handle_t m_adc_handle = nullptr;
        adc_cali_handle_t m_adc_cali_handle = nullptr;

        static constexpr adc_unit_t c_adc_unit = ADC_UNIT_1;
        static constexpr adc_atten_t c_adc_atten = ADC_ATTEN_DB_12;
        static constexpr adc_channel_t c_adc_channel = ADC_CHANNEL_6;
    };
}