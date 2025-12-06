//
// Created on 03-Dec-25.
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#define PHNT_VERSION PHNT_WINDOWS_11_24H2
#include <phnt_windows.h>
#include <phnt.h>
#include <spdlog/spdlog.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <string>
#include <string_view>
#include <utility>
#include <functional>

#define LOG_TRACE SPDLOG_TRACE
#define LOG_DEBUG SPDLOG_DEBUG
#define LOG_INFO SPDLOG_INFO
#define LOG_WARN SPDLOG_WARN
#define LOG_ERROR SPDLOG_ERROR
#define LOG_CRITICAL SPDLOG_CRITICAL

typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned int uint;
typedef float float32;
typedef double float64;
typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;

using absl::Status;
using absl::StatusOr;
using namespace std::literals;
using namespace std::placeholders;
