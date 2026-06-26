#include "components/motor.hpp"

#include <esp_log.h>

namespace client {
    void motor_t::init(gpio_num_t a1, gpio_num_t a2) {
        // configure pwm timer
        ledc_timer_config_t pwm_timer = {};
        pwm_timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
        pwm_timer.duty_resolution = c_motor_pwm_res;
        pwm_timer.timer_num       = LEDC_TIMER_0;
        pwm_timer.freq_hz         = c_motor_pwm_freq;
        pwm_timer.clk_cfg         = LEDC_AUTO_CLK;
        ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer));

        configure_channel(LEDC_CHANNEL_0, a1);
        configure_channel(LEDC_CHANNEL_1, a2);
    }

    void motor_t::set_active_state(bool is_active) {
        m_is_active = is_active;

        if (!is_active) {
            set_motor_direction(motor_direction_t::none);
            m_current_duty = 0; 
        }
    }

    bool motor_t::is_active() const {
        return m_is_active;
    }

    uint32_t motor_t::get_current_duty() const {
        return m_current_duty;
    }

    motor_direction_t motor_t::get_current_direction() const {
        return m_current_direction;
    }

    void motor_t::set_motor_direction(motor_direction_t direction) {
        if (direction == motor_direction_t::clockwise) {
            ESP_ERROR_CHECK(ledc_set_duty(c_motor_pwm_mode, LEDC_CHANNEL_0, m_current_duty));
            ESP_ERROR_CHECK(ledc_set_duty(c_motor_pwm_mode, LEDC_CHANNEL_1, 0));
        }
        else if (direction == motor_direction_t::counter_clockwise) {
            ESP_ERROR_CHECK(ledc_set_duty(c_motor_pwm_mode, LEDC_CHANNEL_0, 0));
            ESP_ERROR_CHECK(ledc_set_duty(c_motor_pwm_mode, LEDC_CHANNEL_1, m_current_duty));
        }
        else if (direction == motor_direction_t::none) {
            ESP_ERROR_CHECK(ledc_set_duty(c_motor_pwm_mode, LEDC_CHANNEL_0, 0));
            ESP_ERROR_CHECK(ledc_set_duty(c_motor_pwm_mode, LEDC_CHANNEL_1, 0));
        }
        else return;

        ESP_ERROR_CHECK(ledc_update_duty(c_motor_pwm_mode, LEDC_CHANNEL_0));
        ESP_ERROR_CHECK(ledc_update_duty(c_motor_pwm_mode, LEDC_CHANNEL_1));

        m_current_direction = direction;
    }

    void motor_t::set_motor_duty(uint32_t duty) {
        if (!m_is_active) {
            ESP_LOGW(c_tag, "Motor is not active");
            return;
        }

        if (duty > c_motor_max_duty) {
            ESP_LOGW(c_tag, "Given duty is more then motor max duty!");
            return;
        }

        m_current_duty = duty;
            
        set_motor_direction(m_current_direction);
    }

    void motor_t::configure_channel(ledc_channel_t channel, gpio_num_t pin) {
        ledc_channel_config_t cfg = {};
        cfg.gpio_num   = pin;
        cfg.speed_mode = c_motor_pwm_mode;
        cfg.channel    = channel;
        cfg.timer_sel  = LEDC_TIMER_0;
        cfg.duty       = 0;
        cfg.hpoint     = 0;

        ESP_ERROR_CHECK(ledc_channel_config(&cfg));
    }
}