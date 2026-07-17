#pragma once

#include <sdkconfig.h>

#include "rtos/mailbox.hpp"
#include "rtos/event_queue.hpp"

#include "common/api/types.hpp"
#include "common/messages/motor.hpp"
#include "tcp/message_handling/tcp_event_data.hpp"

namespace client {
    struct system_bus_t {
        common::esp_id_t train_id = 0;

        mailbox_t<common::rail_id_t> current_rail;
        mailbox_t<common::rail_id_t> station_target;
        mailbox_t<common::motor_control_t> motor_command;

        event_queue_t<tcp_event_data_t> outgoing_messages;

        system_bus_t() : outgoing_messages(CONFIG_TCP_EVENT_QUEUE_SIZE) {
        }
    };
}