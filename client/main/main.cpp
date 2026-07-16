#include "application.hpp"

extern "C" void app_main() {
    static client::application_t app;
    app.run();
}