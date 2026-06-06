cmake_minimum_required(VERSION 3.22...3.30)
project(esp_networking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF) # hush clangd

include(CTest)
enable_testing()

# set build dir
set(BINARY_FOLDER "esp_networking")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${BINARY_FOLDER}"
)

# deps
add_subdirectory(vendors/spdlog)
add_subdirectory(vendors/Catch2)
add_subdirectory(vendors/json)

# internal
add_subdirectory(common)
add_subdirectory(host)
add_subdirectory(tests)

# copy scripts
add_custom_target(copy_scripts ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/scripts
        ${CMAKE_BINARY_DIR}/${BINARY_FOLDER}/scripts
)