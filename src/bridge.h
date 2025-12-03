//
// Created on 03-Dec-25.
// Copyright (c) 2025. All rights reserved.
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
