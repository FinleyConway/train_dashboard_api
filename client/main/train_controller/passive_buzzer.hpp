#pragma once

#include <expected>

#include <esp_err.h>
#include <driver/ledc.h>
#include <driver/gpio.h>

// https://docs.arduino.cc/built-in-examples/digital/toneMelody/

namespace client {
    class passive_buzzer_t {
    public:
        static std::expected<passive_buzzer_t, esp_err_t> create(gpio_num_t pwm, ledc_channel_t pwm_channel) {
            // configure pwm timer
            ledc_timer_config_t pwm_timer = {};
            pwm_timer.speed_mode      = c_pwm_mode;
            pwm_timer.duty_resolution = c_pwm_res;
            pwm_timer.timer_num       = c_timer_num;
            pwm_timer.freq_hz         = 1000;
            pwm_timer.clk_cfg         = LEDC_AUTO_CLK;
            
            if (auto err = ledc_timer_config(&pwm_timer); err != ESP_OK) {
                return std::unexpected(err);
            }

            ledc_channel_config_t cfg = {};
            cfg.gpio_num   = pwm;
            cfg.speed_mode = c_pwm_mode;
            cfg.channel    = pwm_channel;
            cfg.timer_sel  = c_timer_num;
            cfg.duty       = c_max_duty / 2;
            cfg.hpoint     = 0;

            if (auto err = ledc_channel_config(&cfg); err != ESP_OK) {
                return std::unexpected(err);
            }

            return passive_buzzer_t(pwm_channel);
        }

        ~passive_buzzer_t() {
            ledc_stop(c_pwm_mode, m_pwm_channel, 0);
        }

        passive_buzzer_t(const passive_buzzer_t&) = delete;
        passive_buzzer_t& operator=(const passive_buzzer_t&) = delete;

        passive_buzzer_t(passive_buzzer_t&&) = default;
        passive_buzzer_t& operator=(passive_buzzer_t&&) = default;

        esp_err_t stop() {
            return ledc_stop(c_pwm_mode, m_pwm_channel, 0);
        }

        esp_err_t set_tone(uint16_t frequency) {
            if (frequency == 0) {
                return stop();
            }

            esp_err_t err = ledc_set_freq(c_pwm_mode, c_timer_num, frequency);
            if (err != ESP_OK) {
                return err;
            }

            return ledc_update_duty(c_pwm_mode, m_pwm_channel);
        }

        esp_err_t set_tone_delay(uint16_t frequency, TickType_t delay) {
            esp_err_t err = set_tone(frequency);
            if (err != ESP_OK) {
                return err;
            }

            vTaskDelay(delay);

            return err;
        }

    private:
        explicit passive_buzzer_t(ledc_channel_t pwm_channel) : m_pwm_channel(pwm_channel) {
        }

    private:
        ledc_channel_t m_pwm_channel;

        static constexpr ledc_mode_t c_pwm_mode = LEDC_LOW_SPEED_MODE;
        static constexpr ledc_timer_t c_timer_num = LEDC_TIMER_0;
        static constexpr ledc_timer_bit_t c_pwm_res = LEDC_TIMER_13_BIT;
        static constexpr uint16_t c_max_duty = ((1 << 13) - 1);
    };
}