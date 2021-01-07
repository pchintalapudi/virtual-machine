#include <algorithm>
#include <bitset>

#include "../classes/class_header.h"
#include "class_file_io.h"
#include "classloader.h"
#include "loaded_iterators.h"
#include "loader_iterators.h"

using namespace oops::classloading;

namespace {

template <std::uint64_t power_of_two, typename integer>
integer align_to(integer in) {
  static_assert(power_of_two != 0 &&
                (power_of_two & -power_of_two) == power_of_two);
  return (in + power_of_two - 1) & ~(power_of_two - 1);
}
struct class_sizes {
  std::size_t total_class_size;
  std::size_t static_size;
  std::size_t static_pointer_count;
  std::size_t static_double_count;
  std::size_t static_long_count;
  std::size_t static_float_count;
  std::size_t static_int_count;
  std::size_t static_short_count;
  std::size_t static_byte_count;
  std::size_t instance_pointer_count;
  std::size_t instance_double_count;
  std::size_t instance_long_count;
  std::size_t instance_float_count;
  std::size_t instance_int_count;
  std::size_t instance_short_count;
  std::size_t instance_byte_count;
  std::size_t overridden_method_count;
  std::size_t total_virtual_method_count;
  std::size_t export_count;
};

class_sizes compute_class_sizes(
    oops::classloading::class_file_reader &reader,
    std::optional<oops::classes::clazz> primary_superclass);

struct sortable_field {
  const char *name;
  std::uint32_t idx;
  std::uint32_t string_idx;
  std::int32_t length;
  std::uint8_t data_type;
  oops::classes::field_type field_type;
  bool operator==(const sortable_field &other) {
    return name == other.name && length == other.length;
  }
  bool operator<(const sortable_field &other) {
    int cmp = std::memcmp(this->name, other.name,
                          std::min(this->length, other.length));
    return cmp < 0 || (cmp == 0 && this->length < other.length);
  }
};
}  // namespace

std::optional<oops::classes::clazz> classloader::impl_load_class(
    const char *cstr, std::int32_t length) {
  constexpr std::uint32_t superclass_start_index = 7;
  constexpr std::uint32_t magic_number = 0x0095F11E;
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
    auto supcls = this->load_class_unwrapped(superclass.name.string,
                                             superclass.name.length);
    if (!supcls) {
      return {};
    }
    // reader.initialize(cstr, length);
    primary = primary ? primary : supcls;
  }
  class_writer writer;
  auto sizes = compute_class_sizes(reader, primary);
  if (!writer.initialize(metaspace, sizes.total_class_size)) {
    reader.destroy();
    return {};
  }
  {
    // Set all the sizes to start
    std::size_t running_offset = 0;
    running_offset += classes::total_header_size;
    // TODO set bounds
    running_offset += sizes.static_size;
    // Set basic header information
    writer.set_vmt_offset(running_offset);
    running_offset += sizes.total_virtual_method_count * sizeof(void *);
    writer.set_class_import_table_offset(running_offset);
    running_offset += align_to<alignof(void *)>(reader.class_reference_count() *
                                                sizeof(std::uint32_t));
    writer.set_field_import_table_offset(running_offset);
    running_offset += reader.import_count() * sizeof(std::uint32_t) * 2;
    writer.set_export_table_offset(running_offset);
    running_offset += sizes.export_count * sizeof(std::uint32_t) * 2;
    writer.set_method_bytecodes_offset(running_offset);
    running_offset += reader.methods_size();
    writer.set_string_pool_offset(running_offset);
    writer.bulk_load_strings(reader);
    writer.bulk_load_methods(reader);
    if (primary) {
      writer.set_superclass(*primary);
      writer.preload_virtual_method_table(*primary);
    }
    // Pull in class imports
    auto class_import_destinations = writer.class_references();
    auto class_import_sources = reader.class_references();
    std::copy(class_import_sources.begin(), class_import_sources.end(),
              class_import_destinations.begin());
    // Pull in field imports
    auto field_import_destinations = writer.field_references();
    auto field_import_sources = reader.imports();
    std::copy(field_import_sources.begin(), field_import_sources.end(),
              field_import_destinations.begin());
    // Pull in and sort class exports
    auto class_export_destinations = writer.field_references();
    auto class_static_fields = reader.static_field_references();
    auto next_field = class_export_destinations.begin();
    next_field = std::copy(class_static_fields.begin(),
                           class_static_fields.end(), next_field);
    auto class_instance_fields = reader.instance_field_references();
    next_field = std::copy(class_instance_fields.begin(),
                           class_instance_fields.end(), next_field);
    auto class_methods = reader.methods();
    next_field =
        std::copy(class_methods.begin(), class_methods.end(), next_field);
    std::sort(class_export_destinations.begin(),
              class_export_destinations.end());
    // TODO write virtual method table
    // Update class index and superclass tables
    std::uint32_t class_index =
        this->instanceof_table.insert_index(reader.implemented_count());
    for (auto superclass :
         crefs.slice(superclass_start_index,
                     superclass_start_index + reader.implemented_count())) {
      auto supcls = this->load_class_unwrapped(superclass.name.string,
                                               superclass.name.length);
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