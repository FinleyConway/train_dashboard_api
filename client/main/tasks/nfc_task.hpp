#pragma once

#include <freertos/FreeRTOS.h>

#include "common/api/types.hpp"

namespace client {
    class nfc_tag_t;

    class nfc_task_t {
    public:
        static void init(common::esp_id_t id);
        
    private:
        static void run(void* parameters);

        static void process_tag(common::esp_id_t id, const nfc_tag_t& tag);

    private:
        static constexpr const char* c_tag = "nfc_task";
        inline static TaskHandle_t s_handle = nullptr;
    };
} 