//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#pragma once

#if __cplusplus < 202302L
// MSVC users (maybe):
// https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
#error \
    "*** You need C++23 to use this generated SDK. ***" \
"If you are using MSVC, you may need to change your compiler flags. See the link in the comment above."
#endif

#define SDKGEN_NS rlsdk
#define SDKGEN_CONCAT1(A_, B_) A_##B_
#define SDKGEN_CONCAT2(A_, B_) SDKGEN_CONCAT1(A_, B_)
#define SDKGEN_PADFIELD(Size_)                   \
 private:                                        \
  char SDKGEN_CONCAT2(pad_, __COUNTER__)[Size_]; \
                                                 \
 public:

#include <cstdint>
#include <string>
#include <stdexcept>

using game_char = wchar_t;
struct FPointer {
  int32_t A;
  int32_t B;
};

// Forward declarations
struct FScriptDelegate;
struct FNameEntry;
struct FName;
class FString;
class UObject;
class UField;
class UEnum;
class UConst;
class UProperty;
class UStruct;
class UFunction;
class UState;
class UClass;

// Runtime SDK headers
#include "sdk_hash.h"
#include "sdk_callableufunction.h"
#include "sdk_unreal.h"
#include "sdk_rt.h"
#include "sdk_unreal_enums.h"

// Generated headers
// clang-format off
#include "Core_structs.h"
#include "Core_classes.h"
#include "Core_params.h"
#include "Engine_structs.h"
#include "Engine_classes.h"
#include "Engine_params.h"
#include "IpDrv_structs.h"
#include "IpDrv_classes.h"
#include "IpDrv_params.h"
#include "XAudio2_structs.h"
#include "XAudio2_classes.h"
#include "XAudio2_params.h"
#include "GFxUI_structs.h"
#include "GFxUI_classes.h"
#include "GFxUI_params.h"
#include "AkAudio_structs.h"
#include "AkAudio_classes.h"
#include "AkAudio_params.h"
#include "WinDrv_structs.h"
#include "WinDrv_classes.h"
#include "WinDrv_params.h"
#include "OnlineSubsystemEOS_structs.h"
#include "OnlineSubsystemEOS_classes.h"
#include "OnlineSubsystemEOS_params.h"
#include "ProjectX_structs.h"
#include "ProjectX_classes.h"
#include "ProjectX_params.h"
#include "TAGame_structs.h"
#include "TAGame_classes.h"
#include "TAGame_params.h"