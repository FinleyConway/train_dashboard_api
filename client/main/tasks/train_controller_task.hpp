#pragma once

#include <freertos/FreeRTOS.h>

#include "task_events/rail_command.hpp"
#include "task_events/motor_command.hpp"
#include "task_events/station_command.hpp"

#include "common/api/types.hpp"
#include "common/messages/motor.hpp"

namespace client {
    class train_controller_task_t {
    public:
        static void init() {
            if (s_handle != nullptr) {
                vTaskDelete(s_handle);
            }

            xTaskCreate(
                run,
                "train_controller_task_t",
                4096,
                nullptr,
                3,
                &s_handle
            );
        }

    private:
        static void run(void* parameters) {
            common::rail_id_t target_station;
            common::rail_id_t current_station;

            while (true) {
                ESP_LOGI(c_tag, "Wait for a new station request");

                // block until a destination is received.
                station_command_t::receive(target_station);

                // TEMP: to recieve, store and send some info about the train speed
                // unless the server sends two requests, the station and the motor????
                motor_command_t::send(common::motor_control_t {
                    .starting_duty = 600,
                    .target_duty = 1023,
                    .ramp_time_ms = 7500,
                    .is_active = true
                });

                // wait until the destination is reached.
                do {
                    rail_command_t::receive(current_station);
                } 
                while (current_station != target_station);

                // stop motor and do other component stuff
                motor_command_t::send(common::motor_control_t::stop());
            }

            vTaskDelete(nullptr);
        }

    private:
        static constexpr const char* c_tag = "train_controller";
        inline static TaskHandle_t s_handle = nullptr;
    };
}