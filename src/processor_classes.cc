//
// Copyright (c) 2024 nyrpqsqq35
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//

#include "processor_classes.h"

#include "sdk_hash.h"

#include "processor.h"
#include "processor_structs.h"

namespace processor {
namespace classes {

struct GeneratedFunction {
  std::string generated_cpp;
  std::string generated_params;
  std::string generated_source;
};

template <bool InMacro = false>
void ProcessFunction(bridge::Pointer<UObject>& clazz, bridge::Pointer<UFunction>& func,
                     std::vector<GeneratedFunction>& generated_functions) {
  constexpr std::string_view kNewLine = InMacro ? "    \\\n" : "\n";
  // DLOG(INFO) << "Yes function wooo meow " << func->GetFullName();

  const auto our_name = /* "exec" + */ CreateIdentifierName<UFunction, false>(func);
  const auto params_name = CreateIdentifierName(clazz) + "_" + our_name;
  std::stringstream file;
  std::stringstream params_file;
  std::stringstream source_file;
  std::stringstream source_copy_in;
  std::stringstream source_copy_out;
  std::stringstream function_params;
  std::string function_signature;
  std::string return_type = "void";
  std::string return_name;

  params_file << "struct " << params_name << "{\n";
  processor::structs::StandaloneProcessStruct<false>(
      func.get(), params_file, [&](const bridge::Pointer<UProperty>& child) {
        constexpr auto kOutPrefix = "out_"sv;
        constexpr auto kOptionalPrefix = "opt_"sv;

        const auto prop_flags = child->property_flags;
        if (!(prop_flags & CPF_Parm)) return;
        const auto is_out = prop_flags & CPF_OutParm;
        const auto is_optional = prop_flags & CPF_OptionalParm;

        const auto child_name = structs::GetPropertyNameForFunctionGen(child);

        std::string name;
        if (is_optional) name += kOptionalPrefix;
        if (is_out) name += kOutPrefix;
        name += child_name;

        if (prop_flags & CPF_ReturnParm) {
          return_type = GetPropertyCType(child);
          return_name = child_name;
        } else {
          if (child->IsA("Class Core.StrProperty") && !is_out) {
            function_params << "const " << GetPropertyCType(child) << "&";
          } else {
            function_params << GetPropertyCType(child);
          }

          // uint8(&name)[25]
          if (child->array_dim > 1) {
            function_params << "(";
          }
          if (is_out) function_params << "&";
          function_params << " " << name;
          if (child->array_dim > 1) {
            function_params << ")[" << child->array_dim << "], ";
          } else {
            function_params << ", ";
          }
          const auto params_ref = "params." + child_name;

          if (child->IsA("Class Core.BoolProperty") ||
              (child->IsA("Class Core.StrProperty") && !is_out)) {
            source_copy_in << "  " << params_ref << " = " << name << ";" << kNewLine;
          } else {
            // std::memcpy(&{params_ref}, &{name}, sizeof({name}));
            const auto CalculateSizeOf = [&](const std::string& name) -> std::string {
              if (child->array_dim > 1) {
                return std::to_string(child->element_size * child->array_dim);
              } else {
                return std::format("sizeof({})", name);
              }
            };

            source_copy_in << "  std::memcpy(&" << params_ref << ", &" << name << ", "
                           << CalculateSizeOf(name) << ");" << kNewLine;

            if (is_out) {
              // std::memcpy(&{name}, &{params_ref}, sizeof({params_ref}));
              source_copy_out << "  std::memcpy(&" << name << ", &" << params_ref << ", "
                              << CalculateSizeOf(params_ref) << ");" << kNewLine;
            }
          }
        }
      });
  params_file << "};\n";

  if (func->function_flags & FUNC_Static) {
    file << "static ";
  } else {
    // for 0-size non-static member struct
    file << "[[msvc::no_unique_address]] ";
  }
  file << "struct " << our_name << " : ";
  const auto find_name_hash = StringHash<>::Calculate(func->GetFullName());
  const auto is_native = !!(func->function_flags & FUNC_Native) ? "true" : "false";

  std::stringstream cufn;
  if (func->function_flags & FUNC_Static) {
    // prepend ULL to find_name_hash to make it an unsigned long long literal
    cufn << "CallableUFunctionStatic<struct " << params_name << ", " << find_name_hash.value()
         << "ULL, " << is_native << ", " << CreateIdentifierName(clazz) << ">";
  } else {
    cufn << "CallableUFunction<struct " << params_name << ", " << find_name_hash.value() << "ULL, "
         << is_native << ">";
  }

  file << cufn.str() << " ";
  file << " {" << kNewLine << "  ";

  if (func->function_flags & FUNC_Static) {
    file << "static ";
  }
  file << return_type << " ";
  source_file << return_type << " ";

  source_file << CreateIdentifierName(clazz) << "::" << our_name << "::";
  {
    std::stringstream ss;
    // ss << our_name;
    ss << "operator()";
    ss << "(";
    const auto& params_str = function_params.str();
    if (!params_str.empty()) ss << params_str.substr(0, params_str.size() - 2);
    ss << ")";
    function_signature = ss.str();
  }
  file << function_signature << ";" << kNewLine;
  file << "} " << our_name << ";" << kNewLine;

  source_file << function_signature << "{ \n";
  source_file << "  " << params_name << " params{};\n";
  source_file << source_copy_in.str() << "\n";

  // clang format has a stroke on the unlikely attribute here
  // clang-format off
  if (our_name == "Call") { [[unlikely]]
    // Fuck you!!!!
    source_file << "  " << cufn.str() << "::Call(params);\n";
  } else {
    source_file << "  Call(params);" << "\n";
  }
  // clang-format on

  source_file << source_copy_out.str() << "\n";
  if (!return_name.empty()) {
    source_file << "  return params." << return_name << ";\n";
  }
  source_file << "};\n\n";
  generated_functions.push_back({file.str(), params_file.str(), source_file.str()});
}

void ProcessClass(bridge::Pointer<UObject> obj) {
  bridge::Pointer<UClass> uclass(obj.get());

  auto package_obj = obj->GetPackageObject();
  auto& pkg = GetPackage(package_obj);
  std::vector<GeneratedFunction> generated_functions;

  std::stringstream file;
  std::stringstream params_file;
  std::stringstream source_file;

  if (const auto our_full_name = obj->GetFullName();
      our_full_name == "Class Core.Field" || our_full_name == "Class Core.Enum" ||
      our_full_name == "Class Core.Const" || our_full_name == "Class Core.Property" ||
      our_full_name == "Class Core.Struct" || /*our_full_name == "Class Core.Object" ||*/
      our_full_name == "Class Core.Group_ORS" || our_full_name == "Class Core.Function" ||
      our_full_name == "Class Core.Class" || our_full_name == "Class Core.State") {
    return;
  } else if (our_full_name == "Class Core.Object") {
    // Generate a macro with the generated functions into be included our own UObject class in a
    // sdk_unreal.h (before the generated ones)
    if (pkg.ProcessedClass(uclass)) return;
    processor::structs::StandaloneProcessStruct<false, false>(
        obj, file, [&](const bridge::Pointer<UProperty>& child) {
          if (child->IsA("Class Core.Function")) {
            bridge::Pointer<UFunction> meow(child.get());

            // These conflict with functions that exist on our UObject class.
            if (const auto fn_name = meow->name.ToString();
                fn_name == "IsA" || fn_name == "GetPackageName" || fn_name == "FindObject") {
              return;
            }

            ProcessFunction<true>(obj, meow, generated_functions);
          }
        });

    std::stringstream macro;
    macro << "#define " << obj->name.ToString() << "_FUNCS   ";
    for (const auto& [generated_cpp, generated_params, generated_source] : generated_functions) {
      params_file << generated_params;
      source_file << generated_source;
      macro << generated_cpp;
    }
    pkg.generated_macros.push_back(macro.str());
    pkg.generated_classes.push_back({uclass.get(), "", params_file.str(), source_file.str()});

    return;
  }

  if (pkg.ProcessedClass(uclass)) return;

  file << "// Property size: ";
  file << std::hex << std::showbase << uclass->property_size;
  file << std::dec << " (" << uclass->property_size << ")\n";

  const auto our_name = CreateIdentifierName(uclass);

  file << "class " << our_name;

  if (uclass->superfield.IsValid()) {
    file << " : public " << CreateIdentifierName(uclass->superfield);
    file << " /* " << uclass->superfield->GetFullName() << " */";

    if (uclass->superfield->GetPackageObject() == package_obj) {
      if (!pkg.ProcessedClass(uclass->superfield)) {
        ProcessClass(bridge::Pointer<UObject>(uclass->superfield.get()));
      }
    }
  }

  file << " {\n";
  file << "public:\n";

  const auto uclass_fullname = uclass->GetFullName();
  file << "static UClass* StaticClass() {\n";
  file << "  static UClass* clazz = UObject::FindClass("
       << StringHash<>::Calculate(uclass_fullname).value() << "ULL); /* " << uclass_fullname << " */\n";
  file << "  return clazz;\n";
  file << "};\n";

  processor::structs::StandaloneProcessStruct<false, true>(
      obj, file, [&](const bridge::Pointer<UProperty>& child) {
        if (child->IsA("Class Core.Function")) {
          bridge::Pointer<UFunction> meow(child.get());
          ProcessFunction<false>(obj, meow, generated_functions);
        }
      });

  file << "\npublic:\n";
  for (const GeneratedFunction& func : generated_functions) {
    params_file << func.generated_params;
    file << func.generated_cpp;
    source_file << func.generated_source;
  }

  file << "};" << std::endl;

  pkg.generated_classes.push_back({uclass.get(), file.str(), params_file.str(), source_file.str()});
}
}  // namespace classes
}  // namespace processor
