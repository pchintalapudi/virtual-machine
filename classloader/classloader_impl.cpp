#include "class_file_io.h"
#include "classloader.h"
#include "loader_iterators.h"

using namespace oops::classloading;

namespace {
std::size_t compute_class_size(
    oops::classloading::class_file_reader &reader,
    std::optional<oops::classes::clazz> primary_superclass,
    std::array<std::uint32_t, 7> &static_offsets,
    std::array<std::uint32_t, 7> &instance_offsets);
}

std::optional<oops::classes::clazz> classloader::impl_load_class(
    const char *cstr, std::int32_t length) {
  constexpr std::uint32_t superclass_start_index = 7;
  constexpr std::uint16_t magic_number = 0xF11E;
  class_file_reader reader;
  if (!reader.initialize(cstr, length)) {
    return {};
  }
  if (reader.magic_number() != magic_number) {
    return {};
  }
  auto crefs = reader.class_references();
  std::optional<classes::clazz> primary{};
  for (auto superclass :
       crefs.slice(superclass_start_index,
                   reader.implemented_count() + superclass_start_index)) {
    // TODO decide whether memory savings are worth it to destroy + reinitialize
    // this class for each superclass
    // reader.destroy();
    auto supcls = this->load_class_unwrapped(superclass.str, superclass.length);
    if (!supcls) {
      return {};
    }
    // reader.initialize(cstr, length);
    primary = primary ? primary : supcls;
  }
  std::array<std::uint32_t, 7> static_offsets{}, instance_offsets{};
  class_writer writer;
  if (!writer.initialize(metaspace,
                         compute_class_size(reader, primary, static_offsets,
                                            instance_offsets))) {
    reader.destroy();
    return {};
  }
  {
    writer.set_superclass(primary ? *primary : classes::clazz(nullptr));
    writer.set_instance_variable_offsets(instance_offsets);
    writer.set_static_variable_offsets(static_offsets);
    // Update class index and superclass tables
    std::uint32_t class_index =
        this->instanceof_table.insert_index(reader.implemented_count());
    for (auto superclass :
         crefs.slice(superclass_start_index,
                     superclass_start_index + reader.implemented_count())) {
      auto supcls =
          this->load_class_unwrapped(superclass.str, superclass.length);
      this->instanceof_table.insert_superclass(supcls->get_raw());
    }
    if (!this->instanceof_table.commit_superclasses()) {
      this->instanceof_table.pop_last_class();
      goto failure;
    }
    writer.set_class_index(class_index);
    reader.destroy();
    return writer.commit();
  }
failure:
  writer.revoke();
  reader.destroy();
  return {};
}