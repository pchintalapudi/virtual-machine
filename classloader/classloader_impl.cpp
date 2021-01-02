#include "../classes/class_header.h"
#include "class_file_io.h"
#include "classloader.h"
#include "loader_iterators.h"

using namespace oops::classloading;

namespace {
std::size_t compute_class_size(
    oops::classloading::class_file_reader &reader,
    std::optional<oops::classes::clazz> primary_superclass) {
  std::size_t total_class_size_guess = oops::classes::total_header_size;
  // TODO accumulate static memory size
  std::size_t static_memory_size = 0;
  static_memory_size =
      (static_memory_size + sizeof(void *) - 1) & ~(sizeof(void *) - 1ull);
  total_class_size_guess += static_memory_size;  // Static memory size
  // Estimate actual virtual method count (no dedup of overridden methods)
  std::size_t superclass_virtual_method_count =
      primary_superclass ? primary_superclass->virtual_method_count() : 0;
  std::size_t method_count =
      reader.defined_method_count() + superclass_virtual_method_count;
  total_class_size_guess += method_count * sizeof(void *);  // Vtable size
  std::size_t class_count = reader.class_reference_count();
  total_class_size_guess += ((class_count + 1) & ~1ull) *
                            sizeof(std::uint32_t);  // Class import table size
  std::size_t import_count = reader.import_count();
  total_class_size_guess +=
      import_count * sizeof(std::uint32_t) * 2;  // Field import table size
  std::size_t export_count = method_count + reader.instance_field_count() +
                             reader.static_field_count();
  total_class_size_guess +=
      export_count * sizeof(std::uint32_t) * 2;     // Export table size
  total_class_size_guess += reader.methods_size();  // Total method size
  total_class_size_guess += (reader.strings_byte_count() + sizeof(void *) - 1) &
                            (sizeof(void *) - 1);  // Total strings size
  return total_class_size_guess;
}
}  // namespace

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
    auto supcls =
        this->load_class_unwrapped(superclass.string, superclass.length);
    if (!supcls) {
      return {};
    }
    // reader.initialize(cstr, length);
    primary = primary ? primary : supcls;
  }
  class_writer writer;
  if (!writer.initialize(metaspace, compute_class_size(reader, primary))) {
    reader.destroy();
    return {};
  }
  {
    writer.set_superclass(primary ? *primary : classes::clazz(nullptr));
    // Update class index and superclass tables
    std::uint32_t class_index =
        this->instanceof_table.insert_index(reader.implemented_count());
    for (auto superclass :
         crefs.slice(superclass_start_index,
                     superclass_start_index + reader.implemented_count())) {
      auto supcls =
          this->load_class_unwrapped(superclass.string, superclass.length);
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
  writer.revoke(this->metaspace);
  reader.destroy();
  return {};
}