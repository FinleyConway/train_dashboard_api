#pragma once

#include <string>
#include <cstdint>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// https://linuxvox.com/blog/using-exec-to-execute-a-system-command-in-a-new-process/

// Linux only atm
namespace host {
    class mdns_service_t {
    public:
        mdns_service_t() = default;
        ~mdns_service_t() {
            stop();
        }
        mdns_service_t(const mdns_service_t&) = delete;
        mdns_service_t& operator=(const mdns_service_t&) = delete;

        void start(const char* name, const char* port) {
            if (m_pid > 0) return;

            m_pid = fork();

            if (m_pid < 0) return; // should probably add status returns for context

            if (m_pid == 0) {
                char* args[] = {
                    (char*)"python3",
                    (char*)"scripts/dns_broadcaster.py",
                    (char*)name,
                    (char*)port,
                    nullptr
                };

                execvp("python3", args);
                _exit(EXIT_FAILURE);
            }
        }

        void stop() {
            if (m_pid <= 0) return;

            kill(m_pid, SIGTERM);

            int status = 0;

            if (waitpid(m_pid, nullptr, WNOHANG) == m_pid) {
                m_pid = -1;
                return;
            }

            m_pid = -1;
        }

    private:
        pid_t m_pid = -1;
    };
}