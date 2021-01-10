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
  std::array<std::uint32_t, 7> static_counts;
  std::size_t overridden_method_count;
  std::size_t total_virtual_method_count;
  std::size_t export_count;

  std::size_t vmt_offset;
  std::size_t class_import_table_offset;
  std::size_t field_import_table_offset;
  std::size_t export_table_offset;
  std::size_t bytecodes_offset;
  std::size_t string_pool_offset;
};

bool is_virtual_method(const oops::classloading::field &method) {
  return method.data_type & 1;
}

class_sizes compute_class_sizes(
    oops::classloading::class_file_reader &reader,
    std::optional<oops::classes::clazz> primary_superclass) {
  class_sizes out{};
  if (primary_superclass) {
    out.total_virtual_method_count +=
        primary_superclass->virtual_method_count();
  }
  for (auto method : reader.methods()) {
    if (is_virtual_method(method)) {
      if (primary_superclass &&
          primary_superclass->reflect_virtual_method_index(method.name)) {
        out.overridden_method_count++;
      } else {
        out.total_virtual_method_count++;
      }
    }
    out.export_count++;
  }
  for (auto svar : reader.static_field_references()) {
    out.static_counts[svar.data_type]++;
  }
  for (unsigned i = 0; i < 7; i++) {
    out.static_size +=
        out.static_counts[i] *
        oops::classes::datatype_size(static_cast<oops::classes::datatype>(i));
  }
  out.total_class_size += sizeof(oops::classes::class_header);
  out.vmt_offset =
      (out.total_class_size += align_to<alignof(void *)>(out.static_size));
  out.class_import_table_offset =
      (out.total_class_size += out.total_virtual_method_count * sizeof(void *));
  out.field_import_table_offset =
      (out.total_class_size += align_to<alignof(void *)>(
           reader.class_reference_count() * sizeof(std::uint32_t)));
  out.export_table_offset = (out.total_class_size +=
                             reader.import_count() * sizeof(std::uint32_t) * 2);
  out.bytecodes_offset =
      (out.total_class_size += out.export_count * sizeof(std::uint32_t) * 2);
  out.string_pool_offset = (out.total_class_size += reader.methods_size());
  out.total_class_size += reader.strings_byte_count();
  return out;
}

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
  if (!writer.initialize(metaspace,
                         align_to<alignof(void *)>(sizes.total_class_size))) {
    reader.destroy();
    return {};
  }
  {
    // Set basic header information
    writer.set_vmt_offset(sizes.vmt_offset);
    writer.set_class_import_table_offset(sizes.class_import_table_offset);
    writer.set_field_import_table_offset(sizes.field_import_table_offset);
    writer.set_export_table_offset(sizes.export_table_offset);
    writer.set_method_bytecodes_offset(sizes.bytecodes_offset);
    writer.set_string_pool_offset(sizes.string_pool_offset);
    writer.bulk_load_strings(reader);
    writer.bulk_load_methods(reader);
    if (primary) {
      writer.set_superclass(*primary);
      writer.preload_virtual_method_table(*primary);
    }
    // Pull in class imports
    auto class_import_destinations = writer.class_references();
    auto &class_import_sources = crefs;
    std::copy(class_import_sources.begin(), class_import_sources.end(),
              class_import_destinations.begin());
    // Pull in field imports
    auto field_import_destinations = writer.import_references();
    auto field_import_sources = reader.imports();
    std::copy(field_import_sources.begin(), field_import_sources.end(),
              field_import_destinations.begin());
    // Pull in and sort class exports
    auto class_export_destinations = writer.field_references();
    auto next_export = class_export_destinations.begin();
    std::array<std::uint32_t, 7> static_offsets{};
    for (int i = 6; i-- > 0;) {
      static_offsets[i] =
          sizes.static_counts[i + 1] *
          classes::datatype_size(static_cast<classes::datatype>(i + 1));
    }
    auto class_static_fields = reader.static_field_references();
    for (auto static_field : class_static_fields) {
      static_field.data_idx = static_offsets[static_field.data_type];
      static_offsets[static_field.data_type] += classes::datatype_size(
          static_cast<classes::datatype>(static_field.data_type));
      *next_export = static_field;
      ++next_export;
    }
    writer.set_static_variable_offsets(static_offsets);
    std::array<std::uint32_t, 7> instance_counts{};
    if (primary) {
      instance_counts = primary->inherited_variable_counts();
    }
    auto class_instance_fields = reader.instance_field_references();
    for (auto instance_field : class_instance_fields) {
      instance_field.data_idx = instance_counts[instance_field.data_type]++;
      *next_export = instance_field;
      ++next_export;
    }
    writer.set_instance_variable_counts(instance_counts);
    auto class_methods = reader.methods();
    std::size_t default_method_index =
        primary ? primary->virtual_method_count() : 0;
    for (auto method : class_methods) {
      method.data_idx = writer.translate_method_index(method.data_idx);
      if (is_virtual_method(method)) {
        auto override_idx = std::optional<std::uint32_t>();
        if (primary) {
          override_idx = primary->reflect_virtual_method_index(method.name);
        }
        if (!override_idx) {
          override_idx = default_method_index;
          default_method_index += sizeof(void *);
        }
        writer.set_virtual_method(*override_idx, method.data_idx);
      }
      *next_export = method;
      ++next_export;
    }
    std::sort(class_export_destinations.begin(),
              class_export_destinations.end());
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