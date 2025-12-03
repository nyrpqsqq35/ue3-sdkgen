#include "processor_classes.h"

#include "processor.h"
#include "processor_structs.h"

namespace processor {
namespace classes {

struct GeneratedFunction {
  std::string generated_cpp;
  std::string generated_params;
  std::string generated_source;
};

void ProcessFunction(bridge::Pointer<UObject>& clazz, bridge::Pointer<UFunction>& func,
                     std::vector<GeneratedFunction>& generated_functions) {
  // DLOG(INFO) << "Yes function wooo meow " << func->GetFullName();

  const auto our_name = /* "exec" + */ CreateIdentifierName(func);
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
          function_params << GetPropertyCType(child);

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

          if (child->IsA("Class Core.BoolProperty")) {
            source_copy_in << "  " << params_ref << " = " << name << ";\n";
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
                           << CalculateSizeOf(name) << ");\n";

            if (is_out) {
              // std::memcpy(&{name}, &{params_ref}, sizeof({params_ref}));
              source_copy_out << "  std::memcpy(&" << name << ", &" << params_ref << ", "
                              << CalculateSizeOf(params_ref) << ");\n";
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
  file << "struct " << our_name << " {\n  ";

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
  file << function_signature << ";\n";
  file << "} " << our_name << ";\n";

  source_file << function_signature << "{ \n";
  source_file << "  static UFunction* fn = nullptr;\n";
  source_file << "  " << params_name << " params{};\n";
  source_file << source_copy_in.str() << "\n";

  if (func->function_flags & FUNC_Native) {
    source_file << "  fn->function_flags &= ~0x00000400;\n";
  }
  if (func->function_flags & FUNC_Static) {
    source_file << "  " << CreateIdentifierName(clazz) << "::StaticClass()";
  } else {
    // source_file << "  this";
    source_file << "  reinterpret_cast<UObject*>(this)";
  }
  source_file << "->ProcessEvent(fn, &params, nullptr);\n";
  if (func->function_flags & FUNC_Native) {
    source_file << "  fn->function_flags |= 0x00000400;\n";
  }
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

  auto our_full_name = obj->GetFullName();
  if (our_full_name == "Class Core.Field" || our_full_name == "Class Core.Enum" ||
      our_full_name == "Class Core.Const" || our_full_name == "Class Core.Property" ||
      our_full_name == "Class Core.Struct" || our_full_name == "Class Core.Object" ||
      our_full_name == "Class Core.Group_ORS" || our_full_name == "Class Core.Function" ||
      our_full_name == "Class Core.Class" || our_full_name == "Class Core.State")
    return;

  if (pkg.ProcessedClass(uclass)) return;

  std::stringstream file;
  std::stringstream params_file;
  std::stringstream source_file;

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

  file << "static UClass* StaticClass() {\n";
  file << "  static UClass* clazz = UObject::FindClass(\"" << uclass->GetFullName() << "\");\n";
  file << "  return clazz;\n";
  file << "};\n";

  processor::structs::StandaloneProcessStruct<false, true>(
      obj, file, [&](const bridge::Pointer<UProperty>& child) {
        if (child->IsA("Class Core.Function")) {
          // if (child->GetFullName() == "Function TAGame.BTAction_MoveTo.DriveAwayFrom" ||
          //     child->GetFullName() == "Function TAGame.BTAction_MoveTo.GetAimDir" ||
          //     child->GetFullName() == "Function TAGame.EngineShare_TA.DebugDedicatedServer") {
          bridge::Pointer<UFunction> meow(child.get());
          ProcessFunction(obj, meow, generated_functions);
          // }
        }
      });

  {
    // bridge::Pointer<UProperty> child(uclass->children.get());
    // absl::flat_hash_set<std::string> used_names;
    // int member_index = 0;
    // while (child.IsValid()) {
    //   if (child->IsA("Class Core.Property")) {
    //     file << GetPropertyCType(child) << " ";

    //     auto struct_member_name = CreateValidName(child->name.ToString());

    //     file << struct_member_name;
    //     if (used_names.contains(struct_member_name)) {
    //       file << std::dec << member_index;
    //     } else {
    //       used_names.insert(struct_member_name);
    //     }

    //     if (child->array_dim > 1) {
    //       file << "[" << std::dec << std::setw(0) << child->array_dim << "]";
    //     }
    //     if (child->IsA("Class Core.MapProperty")) {
    //       file << "[" << std::dec << std::setw(0) << GetPropertySize(child) << "]";
    //     }

    //     if (child->IsA("Class Core.BoolProperty")) {
    //       // Bitfield
    //       file << " : 1";
    //     }

    //     file << ";";

    //     file << " // " << std::hex << std::showbase << std::setfill('0') << child->offset;
    //     file << " size " << GetPropertySize(child) << "\n";
    //   } else if (child->IsA("Class Core.Function")) {
    //     if (child->GetFullName() == "Function TAGame.BTAction_MoveTo.DriveAwayFrom" ||
    //         child->GetFullName() == "Function TAGame.BTAction_MoveTo.GetAimDir") {
    //       bridge::Pointer<UFunction> meow(child.get());
    //       ProcessFunction(meow, generated_functions);
    //     }
    //   }
    //   child.Set(child->next.get());
    //   ++member_index;
    // }
  }

  file << "\npublic:\n";
  for (const GeneratedFunction& func : generated_functions) {
    params_file << func.generated_params;
    file << func.generated_cpp;
    source_file << func.generated_source;
  }

  file << "};" << std::endl;

  // file << "static_assert(sizeof(" << our_name << ") == " << std::dec << e->property_size << ");"
  //      << std::endl;

  pkg.generated_classes.push_back({uclass.get(), file.str(), params_file.str(), source_file.str()});
}
}  // namespace classes
}  // namespace processor
