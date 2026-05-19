#pragma once

#include <string>
#include <cstdint>
#include <csignal>
#include <cstdlib>
#include <cerrno>
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

        void start(const char* hostname, const char* port) {
            if (m_pid > 0) return;

            m_pid = fork();

            if (m_pid < 0) {
                m_pid = -1;

                return; // should probably add status returns for context
            }

            if (m_pid == 0) {
                // make the child process run the mdns broadcaster
                execlp("python3",
                    "python3",
                    "scripts/dns_broadcaster.py",
                    hostname,
                    port,
                    nullptr
                );

                // exit the child process if an error happens
                _exit(EXIT_FAILURE);
            }
        }

        void stop() {
            if (m_pid <= 0) return;

            // check if child already exited
            pid_t result = waitpid(m_pid, nullptr, WNOHANG);

            if (result == 0) {
                // stop dns service
                kill(m_pid, SIGTERM);

                while (waitpid(m_pid, nullptr, 0) == -1) {
                    if (errno != EINTR) break;
                }
            }

            m_pid = -1;
        }

    private:
        pid_t m_pid = -1;
    };
}