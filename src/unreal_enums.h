//
// Created on 03-Dec-25.
// Copyright (c) 2025. All rights reserved.
//

#pragma once
#include <cstdint>

//
// Flags describing a class.
//
enum EClassFlags : int64_t {
  // Base flags.
  CLASS_None = 0x00000000,
  CLASS_Abstract = 0x00000001,   // Class is abstract and can't be instantiated directly.
  CLASS_Compiled = 0x00000002,   // Script has been compiled successfully.
  CLASS_Config = 0x00000004,     // Load object configuration at construction time.
  CLASS_Transient = 0x00000008,  // This object type can't be saved; null it out at save time.
  CLASS_Parsed = 0x00000010,     // Successfully parsed.
  CLASS_Localized = 0x00000020,  // Class contains localized text.
  CLASS_SafeReplace =
      0x00000040,  // Objects of this class can be safely replaced with default or NULL.
  CLASS_RuntimeStatic = 0x00000080,  // Objects of this class are static during gameplay.
  CLASS_NoExport = 0x00000100,       // Don't export to C++ header.
  CLASS_Placeable = 0x00000200,      // Allow users to create in the editor.
  CLASS_PerObjectConfig =
      0x00000400,  // Handle object configuration on a per-object basis, rather than per-class.
  CLASS_NativeReplication = 0x00000800,  // Replication handled in C++.
  CLASS_EditInlineNew = 0x00001000,      // Class can be constructed from editinline New button.
  CLASS_CollapseCategories =
      0x00002000,                   // Display properties in the editor without using categories.
  CLASS_IsAUProperty = 0x00008000,  // IsA UProperty
  CLASS_IsAUObjectProperty = 0x00010000,  // IsA UObjectProperty
  CLASS_IsAUBoolProperty = 0x00020000,    // IsA UBoolProperty
  CLASS_IsAUState = 0x00040000,           // IsA UState
  CLASS_IsAUFunction = 0x00080000,        // IsA UFunction

  CLASS_NeedsDefProps = 0x00100000,  // Class needs its defaultproperties imported
  CLASS_HasComponents = 0x00400000,  // Class has component properties.

  CLASS_Hidden =
      0x00800000,  // Don't show this class in the editor class browser or edit inline new menus.
  CLASS_Deprecated = 0x01000000,    // Don't save objects of this class when serializing
  CLASS_HideDropDown = 0x02000000,  // Class not shown in editor drop down for class selection

  CLASS_Exported = 0x04000000,  // Class has been exported to a header file

  // Flags to inherit from base class.
  CLASS_Inherit = CLASS_Transient | CLASS_Config | CLASS_Localized | CLASS_SafeReplace |
                  CLASS_RuntimeStatic | CLASS_PerObjectConfig | CLASS_Placeable |
                  CLASS_IsAUProperty | CLASS_IsAUObjectProperty | CLASS_IsAUBoolProperty |
                  CLASS_IsAUState | CLASS_IsAUFunction | CLASS_HasComponents | CLASS_Deprecated,
  CLASS_RecompilerClear = CLASS_Inherit | CLASS_Abstract | CLASS_NoExport | CLASS_NativeReplication,
  CLASS_ScriptInherit = CLASS_Inherit | CLASS_EditInlineNew | CLASS_CollapseCategories,
};

//
// Flags describing an object instance.
//
enum EObjectFlags : int64_t {
  RF_NoFlags = 0x00000000,
  RF_Transactional = 0x00000001,       // Object is transactional.
  RF_Unreachable = 0x00000002,         // Object is not reachable on the object graph.
  RF_Public = 0x00000004,              // Object is visible outside its package.
  RF_TagImp = 0x00000008,              // Temporary import tag in load/save.
  RF_TagExp = 0x00000010,              // Temporary export tag in load/save.
  RF_Obsolete = 0x00000020,            // Object marked as obsolete and should be replaced.
  RF_TagGarbage = 0x00000040,          // Check during garbage collection.
  RF_Final = 0x00000080,               // Object is not visible outside of class.
  RF_PerObjectLocalized = 0x00000100,  // Object is localized by instance name, not by class.
  RF_NeedLoad = 0x00000200,            // During load, indicates object needs loading.
  RF_HighlightedName = 0x00000400,     // A hardcoded name which should be syntax-highlighted.
  RF_EliminateObject = 0x00000400,     // NULL out references to this during garbage collecion.
  RF_InSingularFunc = 0x00000800,      // In a singular function.
  RF_RemappedName = 0x00000800,        // Name is remapped.
  RF_Suppress = 0x00001000,            // warning: Mirrored in UnName.h. Suppressed log name.
  RF_StateChanged = 0x00001000,        // Object did a state change.
  RF_InEndState = 0x00002000,          // Within an EndState call.
  RF_Transient = 0x00004000,           // Don't save object.
  RF_Preloading = 0x00008000,          // Data is being preloaded from file.
  RF_LoadForClient = 0x00010000,       // In-file load for client.
  RF_LoadForServer = 0x00020000,       // In-file load for client.
  RF_LoadForEdit = 0x00040000,         // In-file load for client.
  RF_Standalone = 0x00080000,          // Keep object around for editing even if unreferenced.
  RF_NotForClient = 0x00100000,        // Don't load this object for the game client.
  RF_NotForServer = 0x00200000,        // Don't load this object for the game server.
  RF_NotForEdit = 0x00400000,          // Don't load this object for the editor.
  RF_Destroyed = 0x00800000,           // Object Destroy has already been called.
  RF_NeedPostLoad = 0x01000000,        // Object needs to be postloaded.
  RF_HasStack = 0x02000000,            // Has execution stack.
  RF_Native = 0x04000000,              // Native (UClass only).
  RF_Marked = 0x08000000,              // Marked (for debugging).
  RF_ErrorShutdown = 0x10000000,       // ShutdownAfterError called.
  RF_DebugPostLoad = 0x20000000,       // For debugging Serialize calls.
  RF_DebugSerialize = 0x40000000,      // For debugging Serialize calls.
  RF_DebugDestroy = 0x80000000,        // For debugging Destroy calls.
  RF_EdSelected = 0x80000000,          // Object is selected in one of the editors browser windows.

  // RF_ContextFlags = RF_NotForClient | RF_NotForServer | RF_NotForEdit,  // All context flags.
  // RF_LoadContextFlags =
  // RF_LoadForClient | RF_LoadForServer | RF_LoadForEdit,  // Flags affecting loading.

  // RF_Load = RF_ContextFlags | RF_LoadContextFlags | RF_Public | RF_Final | RF_Standalone |
  //           RF_Native | RF_Obsolete | RF_Transactional | RF_HasStack |
  //           RF_PerObjectLocalized,                          // Flags to load from Unrealfiles.
  // RF_Keep = RF_Native | RF_Marked | RF_PerObjectLocalized,  // Flags to persist across loads.
  // RF_ScriptMask = RF_Transactional | RF_Public | RF_Final | RF_Transient | RF_NotForClient |
  //                 RF_NotForServer | RF_NotForEdit | RF_Standalone  // Script-accessible flags.
};

enum EFunctionFlags : uint64_t {
  FUNC_None = 0x00000000,
  FUNC_Final = 0x00000001,
  FUNC_Defined = 0x00000002,
  FUNC_Iterator = 0x00000004,
  FUNC_Latent = 0x00000008,
  FUNC_PreOperator = 0x00000010,
  FUNC_Singular = 0x00000020,
  FUNC_Net = 0x00000040,
  FUNC_NetReliable = 0x00000080,
  FUNC_Simulated = 0x00000100,
  FUNC_Exec = 0x00000200,
  FUNC_Native = 0x00000400,
  FUNC_Event = 0x00000800,
  FUNC_Operator = 0x00001000,
  FUNC_Static = 0x00002000,
  FUNC_NoExport = 0x00004000,
  FUNC_OptionalParm = 0x00004000,
  FUNC_Const = 0x00008000,
  FUNC_Invariant = 0x00010000,
  FUNC_Public = 0x00020000,
  FUNC_Private = 0x00040000,
  FUNC_Protected = 0x00080000,
  FUNC_Delegate = 0x00100000,
  FUNC_NetServer = 0x00200000,
  FUNC_HasOutParms = 0x00400000,
  FUNC_HasDefaults = 0x00800000,
  FUNC_NetClient = 0x01000000,
  FUNC_DLLImport = 0x02000000,

  FUNC_K2Call = 0x04000000,
  FUNC_K2Override = 0x08000000,
  FUNC_K2Pure = 0x10000000,
  FUNC_EditorOnly = 0x20000000,
  FUNC_Lambda = 0x40000000,
  FUNC_NetValidate = 0x80000000,

  FUNC_FuncInherit = (FUNC_Exec | FUNC_Event),
  FUNC_FuncOverrideMatch =
      (FUNC_Exec | FUNC_Final | FUNC_Latent | FUNC_PreOperator | FUNC_Iterator | FUNC_Static |
       FUNC_Public | FUNC_Protected | FUNC_Const),
  FUNC_NetFuncFlags = (FUNC_Net | FUNC_NetReliable | FUNC_NetServer | FUNC_NetClient),

  FUNC_AllFlags = 0xFFFFFFFF
};

enum EPropertyFlags : uint64_t {
  CPF_Edit = 0x0000000000000001,  // Property is user-settable in the editor.
  CPF_Const =
      0x0000000000000002,  // Actor's property always matches class's default actor property.
  CPF_Input = 0x0000000000000004,          // Variable is writable by the input system.
  CPF_ExportObject = 0x0000000000000008,   // Object can be exported with actor.
  CPF_OptionalParm = 0x0000000000000010,   // Optional parameter (if CPF_Param is set).
  CPF_Net = 0x0000000000000020,            // Property is relevant to network replication.
  CPF_EditFixedSize = 0x0000000000000040,  // Indicates that elements of an array can be modified,
                                           // but its size cannot be changed.
  CPF_Parm = 0x0000000000000080,           // Function/When call parameter.
  CPF_OutParm = 0x0000000000000100,        // Value is copied out after function call.
  CPF_SkipParm = 0x0000000000000200,    // Property is a short-circuitable evaluation function parm.
  CPF_ReturnParm = 0x0000000000000400,  // Return value.
  CPF_CoerceParm = 0x0000000000000800,  // Coerce args into this function parameter.
  CPF_Native =
      0x0000000000001000,  // Property is native: C++ code is responsible for serializing it.
  CPF_Transient =
      0x0000000000002000,  // Property is transient: shouldn't be saved, zero-filled at load time.
  CPF_Config = 0x0000000000004000,        // Property should be loaded/saved as permanent profile.
  CPF_Localized = 0x0000000000008000,     // Property should be loaded as localizable text.
  CPF_Travel = 0x0000000000010000,        // Property travels across levels/servers.
  CPF_EditConst = 0x0000000000020000,     // Property is uneditable in the editor.
  CPF_GlobalConfig = 0x0000000000040000,  // Load config from base class, not subclass.
  CPF_Component = 0x0000000000080000,     // Property containts component references.
  CPF_AlwaysInit =
      0x0000000000100000,  // Property should never be exported as NoInit(@todo - this doesn't need
                           // to be a property flag...only used during make).
  CPF_DuplicateTransient =
      0x0000000000200000,  // Property should always be reset to the default value during any type
                           // of duplication (copy/paste, binary duplication, etc.).
  CPF_NeedCtorLink = 0x0000000000400000,  // Fields need construction/destruction.
  CPF_NoExport =
      0x0000000000800000,  // Property should not be exported to the native class header file.
  CPF_NoClear = 0x0000000002000000,        // Hide clear (and browse) button.
  CPF_EditInline = 0x0000000004000000,     // Edit this object reference inline.
  CPF_EditInlineUse = 0x0000000010000000,  // EditInline with Use button.
  CPF_EditFindable =
      0x0000000008000000,  // References are set by clicking on actors in the editor viewports.
  CPF_Deprecated =
      0x0000000020000000,  // Property is deprecated.  Read it from an archive, but don't save it.
  CPF_DataBinding =
      0x0000000040000000,  // Indicates that this property should be exposed to data stores.
  CPF_SerializeText =
      0x0000000080000000,  // Native property should be serialized as text (ImportText, ExportText).
  CPF_RepNotify = 0x0000000100000000,         // Notify actors when a property is replicated.
  CPF_Interp = 0x0000000200000000,            // Interpolatable property for use with matinee.
  CPF_NonTransactional = 0x0000000400000000,  // Property isn't transacted.
  CPF_EditorOnly = 0x0000000800000000,        // Property should only be loaded in the editor.
  CPF_NotForConsole = 0x0000001000000000,     // Property should not be loaded on console (or be a
                                              // console cooker commandlet).
  CPF_RepRetry = 0x0000002000000000,  // Property replication of this property if it fails to be
                                      // fully sent (e.g. object references not yet available to
                                      // serialize over the network).
  CPF_PrivateWrite =
      0x0000004000000000,  // Property is const outside of the class it was declared in.
  CPF_ProtectedWrite = 0x0000008000000000,     // Property is const outside of the class it was
                                               // declared in and subclasses.
  CPF_ArchetypeProperty = 0x0000010000000000,  // Property should be ignored by archives which have
                                               // ArIgnoreArchetypeRef set.
  CPF_EditHide = 0x0000020000000000,     // Property should never be shown in a properties window.
  CPF_EditTextBox = 0x0000040000000000,  // Property can be edited using a text dialog box.
  CPF_CrossLevelPassive =
      0x0000100000000000,  // Property can point across levels, and will be serialized properly, but
                           // assumes it's target exists in-game (non-editor)
  CPF_CrossLevelActive =
      0x0000200000000000,  // Property can point across levels, and will be serialized properly, and
                           // will be updated when the target is streamed in/out
};

// enum EObjectFlags {
//   RF_NoFlags = 0x00000000,
//   RF_Public = 0x00000001,
//   RF_Standalone = 0x00000002,
//   RF_MarkAsNative = 0x00000004,
//   RF_Transactional = 0x00000008,
//   RF_ClassDefaultObject = 0x00000010,
//   RF_ArchetypeObject = 0x00000020,
//   RF_Transient = 0x00000040,
//   RF_MarkAsRootSet = 0x00000080,
//   RF_TagGarbageTemp = 0x00000100,
//   RF_NeedInitialization = 0x00000200,
//   RF_NeedLoad = 0x00000400,
//   RF_KeepForCooker = 0x00000800,
//   RF_NeedPostLoad = 0x00001000,
//   RF_NeedPostLoadSubobjects = 0x00002000,
//   RF_NewerVersionExists = 0x00004000,
//   RF_BeginDestroyed = 0x00008000,
//   RF_FinishDestroyed = 0x00010000,
//   RF_BeingRegenerated = 0x00020000,
//   RF_DefaultSubObject = 0x00040000,
//   RF_WasLoaded = 0x00080000,
//   RF_TextExportTransient = 0x00100000,
//   RF_LoadCompleted = 0x00200000,
//   RF_InheritableComponentTemplate = 0x00400000,
//   RF_DuplicateTransient = 0x00800000,
//   RF_StrongRefOnFrame = 0x01000000,
//   RF_NonPIEDuplicateTransient = 0x02000000,
//   RF_Dynamic = 0x04000000,
//   RF_WillBeLoaded = 0x08000000,
// };
// #endregion
