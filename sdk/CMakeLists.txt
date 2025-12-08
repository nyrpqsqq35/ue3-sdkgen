add_library(RLSDK STATIC
        sdk.cc sdk.h sdk_config.h sdk_callableufunction.h sdk_callableufunction.cc sdk_hash.h sdk_hash_impl.h sdk_unreal.h sdk_unreal_array.h sdk_unreal_enums.h sdk_rt.cc sdk_rt.h

        Core_classes.cc Engine_classes.cc IpDrv_classes.cc XAudio2_classes.cc GFxUI_classes.cc AkAudio_classes.cc WinDrv_classes.cc OnlineSubsystemEOS_classes.cc ProjectX_classes.cc TAGame_classes.cc

        verifier.cc
)
target_include_directories(RLSDK PUBLIC SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/")
set_target_properties(RLSDK PROPERTIES
        CXX_STANDARD 23
)