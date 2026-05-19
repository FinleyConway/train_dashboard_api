cmake_minimum_required(VERSION 3.22)
project(tests LANGUAGES CXX)

add_executable(unit_tests
    test_packet_registry.cpp
    test_serialisation.cpp
)

target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2WithMain
    common
)

include(Catch)
catch_discover_tests(unit_tests)