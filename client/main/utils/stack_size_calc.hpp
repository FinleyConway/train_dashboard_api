#pragma once

#include <freertos/FreeRTOS.h>

namespace client {
    class stack_size_calc_t {
    public:
        static void get_remaining(size_t allocated_stack) {
            UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);

            printf(
                "Stack remaining: %u words (%u bytes), used: %u words (%u bytes)\n",
                watermark,
                watermark * sizeof(StackType_t),
                allocated_stack - watermark,
                (allocated_stack - watermark) * sizeof(StackType_t)
            );
        }
    };
}