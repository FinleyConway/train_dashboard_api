# esp_networking
A server-client system for communication between a PC server and multiple ESP32 clients simultaneously. It uses a compile-time TCP stream handler design, using [ASIO](https://github.com/chriskohlhoff/asio) for server networking and Unix socket communication and FreeRTOS for the ESP32s.

```c++
// Server
void on_esp_init(common::init_esp_t init_esp) {
    LOG_INFO("Received from esp: {}", init_esp.id);
}

host::tcp_server_t server(...);
server.register_receive_callback<common::init_esp_t, &on_esp_init>();
server.send_to_client<common::init_esp_t>(...);

// Client
void on_esp_init(common::init_esp_t) {
    static bool state = false;
    state = !state;
    gpio_set_level(GPIO_NUM_27, state);
}

client::tcp_client_t client;
client.register_receieve_callback<common::init_esp_t, &on_esp_init>();
client.listen_to_server();
client.send_to_server<common::init_esp_t>(...);
```

## Building and Running
### Client
Uses VSCode PlatformIO IDE extension to build, flash and monitor esp32s.

#### TODO: Be able to flash wifi informations and other important information
---
### Server:

### Windows (Not Support)

### Linux and Mac:
```bash
# Required for automatic network discovery using mdns.
pip install zeroconf
```

```bash
# Cloning and building project
git clone https://github.com/FinleyConway/esp_networking.git --recursive
cd esp_networking
cmake -S . -B build && cmake --build build

# Run server
./build/esp_networking/host
```

## Using Project
