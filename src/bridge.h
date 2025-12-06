//
// Created on 03-Dec-25.
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once

namespace bridge {
extern HANDLE process;
}
#ifdef SDKGEN_INTERNAL
#include "bridge_impl_internal.h"
#else
#include "bridge_impl_external.h"
#endif
