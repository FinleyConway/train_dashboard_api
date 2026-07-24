#include "train_controller/train_controller_task.hpp"

#include <sdkconfig.h>

#include "app/system_bus.hpp"
#include "common/api/types.hpp"
#include "common/messages/motor_control.hpp"

namespace client {
    train_controller_task_t::train_controller_task_t(system_bus_t& bus) : m_bus(bus) {
    }
    
    train_controller_task_t::~train_controller_task_t() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
        }
    }

    void train_controller_task_t::init() {
        if (m_handle != nullptr) {
            return;
        }

        xTaskCreate(
            run_wrapper,
            "train_controller_task_t",
            CONFIG_TRAIN_TASK_STACK, 
            this,
            CONFIG_TRAIN_TASK_PRIORITY, 
            &m_handle
        );
    }

    void train_controller_task_t::run_wrapper(void* parameters) {
        auto* self = static_cast<train_controller_task_t*>(parameters);
        self->run();
    }

    void train_controller_task_t::run() {
        common::rail_id_t target_station;
        common::rail_id_t current_station;

        while (true) {
            ESP_LOGI(c_tag, "Wait for a new station request");

            // block until a destination is received.
            m_bus.station_target.receive(target_station);

            // TEMP: to recieve, store and send some info about the train speed
            // unless the server sends two requests, the station and the motor????
            m_bus.motor_command.send(common::motor_control_t {
                .starting_duty = 600,
                .target_duty = 1023,
                .ramp_time_ms = 7500,
                .is_active = true
            });

            // wait until the destination is reached.
            do {
                m_bus.current_rail.receive(current_station);
            } 
            while (current_station != target_station);

            // stop motor and do other component stuff
            m_bus.motor_command.send(common::motor_control_t::stop());
        }

        vTaskDelete(nullptr);
        m_handle = nullptr;
    }
}