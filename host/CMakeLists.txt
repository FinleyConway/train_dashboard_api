cmake_minimum_required(VERSION 3.22)
project(host LANGUAGES CXX)

add_executable(host
    src/main.cpp
)

target_include_directories(host PRIVATE
    ${CMAKE_SOURCE_DIR}/vendors/asio-1.36.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(host PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(host PRIVATE
    spdlog::spdlog_header_only
    common
)