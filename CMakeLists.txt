cmake_minimum_required(VERSION 3.22...3.30)
project(esp_networking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF) # hush clangd

add_subdirectory(vendors/spdlog)
add_subdirectory(common)

add_executable(esp_networking
    server_gui/src/main.cpp
)

target_include_directories(esp_networking PRIVATE
    ${CMAKE_SOURCE_DIR}/vendors/asio-1.36.0/include
)

target_compile_definitions(esp_networking PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(esp_networking PRIVATE
    spdlog::spdlog_header_only
    common
)