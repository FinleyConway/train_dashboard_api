# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)

idf_component_register(
    SRCS ${app_sources}

    INCLUDE_DIRS
        "."
        "app"
        "motor/modules"
        "motor/tasks"
        "nfc/modules"
        "nfc/parser"
        "nfc/tasks"
        "rtos"
        "tcp"
        "tcp/message_handling"
        "tcp/tasks"
        "train_controller"
        "wifi"
        "wifi/provisioning"
        "../../common/src" # should be fine as common is header only
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)