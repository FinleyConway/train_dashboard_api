#include "tcp/tasks/tcp_manager_task.hpp"

#include <sdkconfig.h>
#include <esp_log.h>

#include "tcp/tcp_client.hpp"
#include "tcp/tasks/tcp_send_task.hpp"
#include "tcp/tasks/tcp_listen_task.hpp"

namespace client {
    tcp_manager_task_t::tcp_manager_task_t(system_bus_t& system_bus, void* ctx, connection_callback_t callback) : 
        m_tcp_read_task(system_bus, *this),
        m_tcp_listen_task(system_bus, *this)
    {
        tcp_message_handler_t::init(system_bus, ctx, callback);
    }

    tcp_manager_task_t::~tcp_manager_task_t() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
            m_handle = nullptr;
        }
    }

    void tcp_manager_task_t::init() {
        xTaskCreate(
            run_wrapper,
            "tcp_task_t",
            CONFIG_TCP_TASK_STACK, 
            this,
            CONFIG_TCP_TASK_PRIORITY,
            &m_handle
        );
    }

    TaskHandle_t tcp_manager_task_t::get_handle() {
        return m_handle;
    }

    tcp_client_t& tcp_manager_task_t::get_client() {
        return m_client;
    }

    void tcp_manager_task_t::run_wrapper(void* parameters) {
        auto* self = static_cast<tcp_manager_task_t*>(parameters);
        self->run();
    }

    void tcp_manager_task_t::run() {
        tcp_message_handler_t::register_messages(m_client);

        while (true) {
            // connect to the server
            try_connect();

            // create tcp i/o tasks
            m_tcp_read_task.init();
            m_tcp_listen_task.init();

            // listen for if any i/o tasks failures
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            ESP_LOGI(c_tag, "Connection lost");

            m_tcp_read_task.destroy();
            m_tcp_listen_task.destroy();

            m_client.disconnect();
        }

        vTaskDelete(nullptr);
        m_handle = nullptr;
    }

    void tcp_manager_task_t::try_connect() {
        constexpr TickType_t retry_delay = pdMS_TO_TICKS(5000);
        
        ESP_LOGI(c_tag, "Connecting...");

        while (m_client.try_connect(CONFIG_TCP_TIMEOUT) != tcp_status_t::success) {
            vTaskDelay(retry_delay);
        }

        ESP_LOGI(c_tag, "Connected");
    }
}
