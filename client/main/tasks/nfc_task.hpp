#pragma once

#include <freertos/FreeRTOS.h>

#include "components/nfc_reader.hpp"

namespace client {
    struct system_bus_t;
    class nfc_tag_t;

    class nfc_task_t {
    public:
        explicit nfc_task_t(system_bus_t& bus);
        ~nfc_task_t();

        nfc_task_t(const nfc_task_t&) = delete;
        nfc_task_t& operator=(const nfc_task_t&) = delete;

        nfc_task_t(nfc_task_t&&) noexcept = delete;
        nfc_task_t& operator=(nfc_task_t&&) noexcept = delete;

    public:
        void init();
        
    private:
        static void run_wrapper(void* parameters);

        void run();

        void process_tag(const nfc_tag_t& tag);

    private:
        static constexpr const char* c_tag = "nfc_task";

        nfc_reader_t m_nfc_reader;
        system_bus_t& m_bus;
        TaskHandle_t m_handle = nullptr;
    };
} 