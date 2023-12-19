#pragma once

#include "rde/Core.hpp"

#include <fmt/format.h>

#define RDE_INFO(...) ::rde::SendDebugMsg(this, ::rde::DebugLevel::Info, fmt::format(__VA_ARGS__))
#define RDE_WARN(...) ::rde::SendDebugMsg(this, ::rde::DebugLevel::Warn, fmt::format(__VA_ARGS__))
#define RDE_ERROR(...) ::rde::SendDebugMsg(this, ::rde::DebugLevel::Error, fmt::format(__VA_ARGS__))