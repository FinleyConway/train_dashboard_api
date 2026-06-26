#pragma once

#include <driver/ledc.h>
#include <driver/gpio.h>

namespace client {
    enum class motor_direction_t {
        none,
        clockwise,
        counter_clockwise
    };

    class motor_t {
    public:
        motor_t() = default;

        motor_t(const motor_t&) = delete;
        motor_t& operator=(const motor_t&) = delete;

        motor_t(motor_t&&) noexcept = default;
        motor_t& operator=(motor_t&&) noexcept = default;

    public:
        void init(gpio_num_t a1, gpio_num_t a2);

        void set_active_state(bool is_active);

        bool is_active() const;

        uint32_t get_current_duty() const;

        motor_direction_t get_current_direction() const;

        void set_motor_direction(motor_direction_t direction);

        void set_motor_duty(uint32_t duty);
    
    private:
        void configure_channel(ledc_channel_t channel, gpio_num_t pin);

    private:
        motor_direction_t m_current_direction = motor_direction_t::clockwise;
        uint32_t m_current_duty = 0;
        bool m_is_active = true;

        static constexpr const char* c_tag = "motor";
        static constexpr uint16_t c_motor_pwm_freq = 18000;
        static constexpr ledc_mode_t c_motor_pwm_mode = LEDC_HIGH_SPEED_MODE;
        static constexpr ledc_timer_bit_t c_motor_pwm_res = LEDC_TIMER_10_BIT;
        static constexpr uint32_t c_motor_max_duty = ((1 << 10) - 1);
    };
}