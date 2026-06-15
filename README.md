# esp_networking

A type-safe networking framework for communication between a PC host and multiple ESP32 devices. Messages are registered at compile time, allowing automatic serialization, deserialization, and packet dispatch with minimal runtime overhead.

```c++
host::tcp_server_t server;

server.register_receive_callback<common::temperature_t>(
    [](const auto& msg) {
        LOG_INFO("Temperature: {}", msg.value);
    }
);
```


## Building and Running
First, fork this repository. It is intended to be extended and customized through forks rather than used directly alongside the upstream project.

> [!NOTE]
> This repository contains two separate projects that should be built in their own workspaces. This is required because the ESP-IDF environment conflicts with the host build environment.

### Client
Uses esp-idf to build, flash and monitor esp32s.

Follow the [Get Started](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) provided by Espressif to get the tools needed to use this project.

```bash
# building project
idf.py build flash
idf.py monitor # to monitor logs being printed to console
```

#### Flashing WiFi Credentials
If no WiFi credentials are stored in the NVS partition, the device will start a SoftAP that can be joined from another device (e.g. a mobile phone).

It exposes a HTTP endpoint which accepts a POST request:
```
# typically 192.168.4.1
192.168.4.1/wifi_cred/connect
```

```
# body example
{
    "ssid": "network_name",
    "password": "super_secret_password"
}
```
It also exposes a GET endpoint to know when the wifi has been connected (this endpoint should be polled until a final connection state is returned):
```
# typically 192.168.4.1
192.168.4.1/wifi_cred/status
```

```
{
    "status": "connected" | "bad_credentials" | "connecting" | "idle"
}
```

Finally, you can also ping the esp to check if you are connected to the softAP:
```
# typically 192.168.4.1
192.168.4.1/ping
```

---
### Server:

### Windows (Not Supported): 
See: [mdns_service.hpp](host/src/host/protocol/mdns_service.hpp) to add support.

### Linux and Mac:
```bash
# Cloning forked project
git clone https://github.com/{user}/{fork_project_name}.git --recursive # recursive to get its submodules
cd {fork_project_name}

# Setup python env
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# Build and run server
cmake -S . -B build && cmake --build build
./build/esp_networking/host
```

## Using Project
### Project Example:
A [train dashboard API](https://github.com/FinleyConway/train_dashboard_api) that wraps the TCP service in an HTTP server, allowing trains to be monitored and controlled from external devices.

### Creating a message

```cpp
// inside common/src/common/messages

// typical message boilerplate
namespace common {
    struct temperature_t {
        int8_t value;

        static void serialise(std::span<uint8_t>& payload, const temperature_t& data) {
            serialise_t::write(payload, data.value);
        }

        static temperature_t deserialise(std::span<uint8_t> payload) {
            temperature_t result;

            result.value = serialise_t::read<int8_t>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<int8_t>();
        }
    };
}

// inside common/src/common/api/registry.hpp
namespace common {
    using registry_t = packet_registry_impl_t<
        temperature_t // add message to registry template list
    >;
}
```

### Sending and receiving a message
```cpp
// host
host::tcp_server_t server;

server.register_receive_callback<common::temperature_t>(
    [](const auto& temp) {
        LOG_INFO("Current room temp: {}", temp.value);
    }
);

server.send_to_client(esp_id, common::sprinkler_t {
    .active = true
});

// client
static void on_sprinkler(const common::sprinkler_t& data) {
    ESP_LOGI("is_active: %d", data.active);
}

client::tcp_client_t client;

client.register_receieve_callback<common::sprinkler_t, &on_sprinkler>(); 

client.send_to_server(common::temperature_t {
    .value = 30
});
```

### Creating an mDNS service

```cpp
// inside common/src/common/api/service_config.hpp
namespace common {
    struct service_entry_t {
        const char* service_type;
        const char* hostname;
        const char* port;

        constexpr service_entry_t(const char* service_type, const char* hostname, const char* port) 
            : service_type(service_type), hostname(hostname), port(port) 
        {
        }
    };

    struct service_config_t {
        // each service_type name must be unique!
        static constexpr std::array services {
            service_entry_t("esp-tcp", "esp_server.local", "8080"),
            service_entry_t("esp-api", "esp_api.local", "80"), 
        };

        // for tcp server/client to easily get
        static constexpr const char* tcp_hostname = services[0].hostname;
        static constexpr const char* tcp_port = services[0].port;
    };
}
```

Simply add another entry to services and it will automatically start when tcp_server_t starts

```bash
finley@fedora:~$ ping esp_server.local
PING esp_server.local (192.168.1.95) 56(84) bytes of data.
64 bytes from fedora (192.168.1.95): icmp_seq=1 ttl=64 time=0.016 ms
64 bytes from fedora (192.168.1.95): icmp_seq=2 ttl=64 time=0.024 ms
64 bytes from fedora (192.168.1.95): icmp_seq=3 ttl=64 time=0.028 ms
64 bytes from fedora (192.168.1.95): icmp_seq=4 ttl=64 time=0.237 ms
```

## Updating From Upstream
Add this repository as an upstream remote which allows you to fetch and merge with your worked repository

```bash
git remote add upstream https://github.com/FinleyConway/esp_networking.git
```

To update:
```bash
git fetch upstream
git merge upstream/main

# or if rebase is preferred

git fetch upstream
git rebase upstream/main
```