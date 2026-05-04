cmake_minimum_required(VERSION 3.22...3.30)
project(common LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF) # hush clangd

add_library(common STATIC
    src/packet_registry.hpp
    src/registry.hpp

    src/types/restart_esp.hpp
    src/types/init_esp.hpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
