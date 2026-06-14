#pragma once

#include <string>
#include <cstdint>
#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common/api/service_config.hpp"

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

        void start(auto service_config) {
            if (m_pid > 0) return;

            m_pid = fork();

            if (m_pid < 0) {
                m_pid = -1;

                return; // should probably add status returns for context
            }

            if (m_pid == 0) {
                std::vector<const char*> args;

                args.emplace_back("python3");
                args.emplace_back("scripts/dns_broadcaster.py");

                for (const auto& entry : service_config) {
                    args.emplace_back(entry.service_type);
                    args.emplace_back(entry.hostname);
                    args.emplace_back(entry.port);
                }

                args.emplace_back(nullptr);
                
                // make the child process run the mdns broadcaster
                // gotta love the const casting :(
                execvp("python3", const_cast<char* const*>(args.data()));

                perror("execvp failed");

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