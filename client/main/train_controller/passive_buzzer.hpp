#pragma once

#include <driver/ledc.h>
#include <driver/gpio.h>

// https://docs.arduino.cc/built-in-examples/digital/toneMelody/

namespace client {
    class passive_buzzer_t {
    public:
        void init(gpio_num_t pwm, ledc_channel_t pwm_channel) {
            m_pwm_channel = pwm_channel;

            // configure pwm timer
            ledc_timer_config_t pwm_timer = {};
            pwm_timer.speed_mode      = c_pwm_mode;
            pwm_timer.duty_resolution = c_pwm_res;
            pwm_timer.timer_num       = c_timer_num;
            pwm_timer.freq_hz         = 1000;
            pwm_timer.clk_cfg         = LEDC_AUTO_CLK;
            ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer));

            ledc_channel_config_t cfg = {};
            cfg.gpio_num   = pwm;
            cfg.speed_mode = c_pwm_mode;
            cfg.channel    = pwm_channel;
            cfg.timer_sel  = c_timer_num;
            cfg.duty       = c_max_duty / 2;
            cfg.hpoint     = 0;
            ESP_ERROR_CHECK(ledc_channel_config(&cfg));
        }

        esp_err_t set_tone(uint32_t frequency) {
            if (frequency == 0) {
                return ledc_stop(c_pwm_mode, m_pwm_channel, 0);
            }

            return ledc_set_freq(c_pwm_mode, c_timer_num, frequency);
        }

        esp_err_t set_tone_delay(uint32_t frequency, TickType_t delay) {
            esp_err_t err = set_tone(frequency);
            vTaskDelay(delay);

            return err;
        }

    private:
        ledc_channel_t m_pwm_channel;

        static constexpr ledc_mode_t c_pwm_mode = LEDC_LOW_SPEED_MODE;
        static constexpr ledc_timer_t c_timer_num = LEDC_TIMER_0;
        static constexpr ledc_timer_bit_t c_pwm_res = LEDC_TIMER_13_BIT;
        static constexpr uint32_t c_max_duty = ((1 << 13) - 1);
    };
}