cmake_minimum_required(VERSION 3.22)
project(tests LANGUAGES CXX)

add_executable(unit_tests
    test_packet_registry.cpp
)

target_link_libraries(unit_tests PRIVATE
    common
)