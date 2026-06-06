#pragma once

#include <driver/ledc.h>
#include <driver/gpio.h>
#include <esp_log.h>

#define MOTOR_PWM_FREQ 18000
#define MOTOR_PWM_MODE LEDC_HIGH_SPEED_MODE
#define MOTOR_PWM_RES LEDC_TIMER_10_BIT 
#define MOTOR_MAX_DUTY ((1 << 10) - 1)

namespace client {
    enum class motor_direction_t {
        none,
        clockwise,
        counter_clockwise
    };

    class motor_t {
    public:
        motor_t() = default;

        void init(gpio_num_t a1, gpio_num_t a2) {
            // configure pwm timer
            ledc_timer_config_t pwm_timer = {};
            pwm_timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
            pwm_timer.duty_resolution = MOTOR_PWM_RES;
            pwm_timer.timer_num       = LEDC_TIMER_0;
            pwm_timer.freq_hz         = MOTOR_PWM_FREQ;
            pwm_timer.clk_cfg         = LEDC_AUTO_CLK;
            ledc_timer_config(&pwm_timer);

            configure_channel(LEDC_CHANNEL_0, a1);
            configure_channel(LEDC_CHANNEL_1, a2);
        }

        void set_active_state(bool is_active) {
            m_is_active = is_active;

            if (!is_active) {
                set_motor_direction(motor_direction_t::none);
                m_current_duty = 0; 
            }
        }

        bool is_active() const {
            return m_is_active;
        }

        uint32_t get_current_duty() const {
            return m_current_duty;
        }

        motor_direction_t get_current_direction() const {
            return m_current_direction;
        }

        void set_motor_direction(motor_direction_t direction) {
            if (direction == motor_direction_t::clockwise) {
                ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0, m_current_duty);
                ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1, 0);
            }
            else if (direction == motor_direction_t::counter_clockwise) {
                ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0, 0);
                ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1, m_current_duty);
            }
            else if (direction == motor_direction_t::none) {
                ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0, 0);
                ledc_set_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1, 0);
            }
            else return;

            ledc_update_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_0);
            ledc_update_duty(MOTOR_PWM_MODE, LEDC_CHANNEL_1);

            m_current_direction = direction;
        }

        // TT motor seems to be only min of 750 before it starts turinng
        void set_motor_duty(uint32_t duty) {
            if (!m_is_active) {
                ESP_LOGW("MOTOR", "Motor is not active");
                return;
            }

            if (duty > MOTOR_MAX_DUTY) {
                ESP_LOGW("MOTOR", "Given duty is more then motor max duty!");
                return;
            }

            m_current_duty = duty;
                
            set_motor_direction(m_current_direction);
        }
    
    private:
        void configure_channel(ledc_channel_t channel, gpio_num_t pin) {
            ledc_channel_config_t cfg = {};
            cfg.gpio_num   = pin;
            cfg.speed_mode = MOTOR_PWM_MODE;
            cfg.channel    = channel;
            cfg.timer_sel  = LEDC_TIMER_0;
            cfg.duty       = 0;
            cfg.hpoint     = 0;

            ESP_ERROR_CHECK(ledc_channel_config(&cfg));
        }

    private:
        motor_direction_t m_current_direction = motor_direction_t::clockwise;
        uint32_t m_current_duty = 0;
        bool m_is_active = true;
    };
}