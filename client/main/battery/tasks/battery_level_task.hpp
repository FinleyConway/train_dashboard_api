#pragma once

#include <freertos/FreeRTOS.h>
#include <esp_adc/adc_oneshot.h>
#include <sdkconfig.h>

// https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/peripherals/adc.html

namespace client {
    class battery_level_task_t {
    public:
        explicit battery_level_task_t(system_bus_t& bus) : m_bus(bus) {

        }
        ~battery_level_task_t() {
            if (m_handle != nullptr) {
                vTaskDelete(m_handle);
            }            
        }

        battery_level_task_t(const battery_level_task_t&) = delete;
        battery_level_task_t& operator=(const battery_level_task_t&) = delete;

        battery_level_task_t(battery_level_task_t&&) noexcept = delete;
        battery_level_task_t& operator=(battery_level_task_t&&) noexcept = delete;
    
    public:
        void init() {
            xTaskCreate(
                run_wrapper,
                "battery_level_task_t",
                4096,//CONFIG_BATTERY_TASK_STACK,
                this,
                CONFIG_BATTERY_TASK_PRIORITY,
                &m_handle
            );
        }
    private:
        static void run_wrapper(void* paramaters) {
            auto* self = static_cast<battery_level_task_t*>(paramaters);
            self->run();
        }
    
        void run() {
            setup_adc();

            while (true) {
                const int millivolts = get_adc_millivolts();
                const float percentage = get_percentage(millivolts);

                // send battery status to server
                m_bus.outgoing_messages.send(tcp_event_data_t {
                    common::battery_status_t {
                        .id = m_bus.train_id,
                        .percentage_level = percentage
                    }}
                );

                ESP_LOGI("battery_task", "Battery is current (mV): %d full", millivolts);
                ESP_LOGI("battery_task", "Battery is current: %.1f% full", percentage);

                vTaskDelay(pdMS_TO_TICKS(300000)); // block thread for 5 minutes
            }
        }

        static constexpr int battery_to_adc_voltage(float voltage) {
            // from voltage divider
            constexpr uint8_t resistor_100 = 100; // from V
            constexpr uint8_t resistor_10 = 10; // to gnd

            // get the voltage drop value
            const float range = voltage * resistor_10 / (resistor_100 + resistor_10);

            // convert to millivolts
            return static_cast<int>(range * 1000);
        }

        float get_percentage(int voltage) const {
            // min and max millivolts of the train battery
            constexpr int min_voltage = battery_to_adc_voltage(CONFIG_BATTERY_MINIMUM_SAFE);
            constexpr int max_voltage = battery_to_adc_voltage(CONFIG_BATTERY_FULLY_CHARGED);

            const float level = (
                static_cast<float>(voltage - min_voltage) /
                static_cast<float>(max_voltage - min_voltage) *
                100.0f
            );

            return std::clamp(level, 0.0f, 100.0f);
        }

        void setup_adc()
        {
            adc_oneshot_unit_init_cfg_t init_cfg;
            init_cfg.unit_id = static_cast<adc_unit_t>(CONFIG_BATTERY_ADC_UNIT);
            init_cfg.clk_src = ADC_RTC_CLK_SRC_DEFAULT;

            ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &m_adc_handle));

            adc_oneshot_chan_cfg_t chan_cfg = {
                .atten = c_adc_atten,
                .bitwidth = ADC_BITWIDTH_DEFAULT,
            };

            ESP_ERROR_CHECK(adc_oneshot_config_channel(
                m_adc_handle,
                static_cast<adc_channel_t>(CONFIG_BATTERY_ADC_CHANNEL),
                &chan_cfg
            ));

            adc_cali_line_fitting_config_t cali_config;
            cali_config.unit_id = ADC_UNIT_1;
            cali_config.atten = c_adc_atten;
            cali_config.bitwidth = ADC_BITWIDTH_DEFAULT;

            ESP_ERROR_CHECK(
                adc_cali_create_scheme_line_fitting(
                    &cali_config,
                    &m_adc_cali_handle
                )
            );
        }

        int get_adc_millivolts() {
            const uint8_t samples = 128;
            int32_t raw_total = 0;

            for (uint8_t i = 0; i < samples; i++) {
                int raw = 0;

                ESP_ERROR_CHECK(
                    adc_oneshot_read(
                        m_adc_handle,
                        static_cast<adc_channel_t>(CONFIG_BATTERY_ADC_CHANNEL),
                        &raw
                    )
                );

                raw_total += raw;

                vTaskDelay(pdMS_TO_TICKS(1));
            }

            const int32_t raw_average = raw_total / samples;
            int voltage = 0;

            ESP_ERROR_CHECK(
                adc_cali_raw_to_voltage(
                    m_adc_cali_handle,
                    raw_average,
                    &voltage
                )
            );

            return voltage;
        }

    private:
        system_bus_t& m_bus;

        TaskHandle_t m_handle = nullptr;
        adc_oneshot_unit_handle_t m_adc_handle = nullptr;
        adc_cali_handle_t m_adc_cali_handle = nullptr;

        static constexpr adc_atten_t c_adc_atten = ADC_ATTEN_DB_12;
    };
}