#include "processor.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>

#include <filesystem>

#include "bridge.h"
#include "unreal.h"

namespace processor {

absl::node_hash_map<bridge::Pointer<UObject>, Package> Packages;

Package& GetPackage(const bridge::Pointer<UObject>& package) { return Packages[package]; }
const absl::node_hash_map<bridge::Pointer<UObject>, Package>& GetAllPackages() { return Packages; }

void CreatePackage(bridge::Pointer<UObject>& package) {
  std::filesystem::create_directory(sdk_path);

  Packages.insert({package, Package()});
}
bool HasPackageBeenCreated(bridge::Pointer<UObject>& package) { return Packages.contains(package); }

}  // namespace processor
