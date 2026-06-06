#pragma once

#include <csignal>

#include "host/logging/logger.hpp"

#define DEBUG_BREAK() raise(SIGTRAP)
#define LOG_ASSERT(x, ...) if(!(x)) { LOG_ERROR("Assertion failed: {0}. Message: {1}.\nAt: {2}, Line: {3}", #x, __VA_ARGS__, __FILE__, __LINE__); DEBUG_BREAK(); }