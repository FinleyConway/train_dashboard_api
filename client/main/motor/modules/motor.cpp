#include "motor/modules/motor.hpp"

#include <esp_log.h>

namespace client {
    motor_t::~motor_t() {
        ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_reset_pin(m_gpio.ain_a));
        ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_reset_pin(m_gpio.ain_b));
        ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_reset_pin(m_gpio.standby));

        ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_stop(c_motor_pwm_mode, m_gpio.pwm_channel, 0));
    }

    void motor_t::init(const motor_gpio_t& gpio) {
        m_gpio = gpio;
        
        // configure pwm timer
        ledc_timer_config_t pwm_timer = {};
        pwm_timer.speed_mode      = c_motor_pwm_mode;
        pwm_timer.duty_resolution = c_motor_pwm_res;
        pwm_timer.timer_num       = LEDC_TIMER_0;
        pwm_timer.freq_hz         = c_motor_pwm_freq;
        pwm_timer.clk_cfg         = LEDC_AUTO_CLK;
        ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer));

        ledc_channel_config_t cfg = {};
        cfg.gpio_num   = gpio.pwm;
        cfg.speed_mode = c_motor_pwm_mode;
        cfg.channel    = gpio.pwm_channel;
        cfg.timer_sel  = LEDC_TIMER_0;
        cfg.duty       = 0;
        cfg.hpoint     = 0;
        ESP_ERROR_CHECK(ledc_channel_config(&cfg));

        // setup driver 
        ESP_ERROR_CHECK(gpio_set_direction(gpio.ain_a, GPIO_MODE_OUTPUT));
        ESP_ERROR_CHECK(gpio_set_direction(gpio.ain_b, GPIO_MODE_OUTPUT));
        ESP_ERROR_CHECK(gpio_set_direction(gpio.standby, GPIO_MODE_OUTPUT));

        // enable driver
        ESP_ERROR_CHECK(gpio_set_level(gpio.standby, true));
    }

    void motor_t::set_active_state(bool is_active) {
        m_is_active = is_active;

        ESP_ERROR_CHECK(gpio_set_level(m_gpio.standby, is_active));

        ESP_LOGI(c_tag, "Motor active: %d", is_active);

        if (!is_active) {
            set_motor_direction(motor_direction_t::none);
            
            ESP_ERROR_CHECK(ledc_set_duty(c_motor_pwm_mode, m_gpio.pwm_channel, m_current_duty));
            ESP_ERROR_CHECK(ledc_update_duty(c_motor_pwm_mode, m_gpio.pwm_channel));
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
            ESP_ERROR_CHECK(gpio_set_level(m_gpio.ain_a, true));
            ESP_ERROR_CHECK(gpio_set_level(m_gpio.ain_b, false));
        }
        else if (direction == motor_direction_t::counter_clockwise) {
            ESP_ERROR_CHECK(gpio_set_level(m_gpio.ain_a, false));
            ESP_ERROR_CHECK(gpio_set_level(m_gpio.ain_b, true));
        }
        else if (direction == motor_direction_t::none) {
            ESP_ERROR_CHECK(gpio_set_level(m_gpio.ain_a, false));
            ESP_ERROR_CHECK(gpio_set_level(m_gpio.ain_b, false));
        }
        else return;

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

        ESP_ERROR_CHECK(ledc_set_duty(c_motor_pwm_mode, m_gpio.pwm_channel, m_current_duty));
        ESP_ERROR_CHECK(ledc_update_duty(c_motor_pwm_mode, m_gpio.pwm_channel));
    }
}