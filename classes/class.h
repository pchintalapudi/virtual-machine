#ifndef OOPS_CLASSES_CLASS_H
#define OOPS_CLASSES_CLASS_H

#include <optional>
#include <variant>

#include "../gc/iterable.h"
#include "../memory/byteblock.h"
#include "../methods/method.h"
#include "object.h"
#include "../classloader/raw_string.h"

namespace oops {
namespace methods {
class method;
}
namespace gc {
class class_string_iterator;
class class_static_pointer_iterator;
}  // namespace gc
namespace classes {

class clazz;
class string;
typedef std::variant<classloading::raw_string, clazz> class_import;
struct field_import;
struct static_method_import;
struct virtual_method_import;
struct dynamic_method_import;
enum class datatype;

class clazz {
 private:
  memory::byteblock<> class_data;

  std::optional<std::uint32_t> reflect_index(classloading::raw_string str);
  std::optional<std::pair<clazz, std::uint32_t>> reflect_index_recursively(classloading::raw_string str);

 public:
  clazz(void *cls);

  const void *get_raw() const { return class_data.get_raw(); }

  void *get_raw() { return class_data.get_raw(); }

  std::optional<clazz> superclass() const;

  std::uint32_t object_size() const;
  std::uint32_t static_pointer_offset() const;
  std::uint32_t static_double_offset() const;
  std::uint32_t static_long_offset() const;
  std::uint32_t static_float_offset() const;
  std::uint32_t static_int_offset() const;
  std::uint32_t static_short_offset() const;
  std::uint32_t static_byte_offset() const;
  std::uint32_t instance_pointer_offset() const;
  std::uint32_t instance_double_offset() const;
  std::uint32_t instance_long_offset() const;
  std::uint32_t instance_float_offset() const;
  std::uint32_t instance_int_offset() const;
  std::uint32_t instance_short_offset() const;
  std::uint32_t instance_byte_offset() const;

  std::uint32_t get_instance_offset(std::uint32_t internal_offset, datatype dt);

  std::optional<field_import> get_field_import(std::uint32_t index);
  std::optional<class_import> get_class_import(std::uint32_t index);
  std::optional<static_method_import> get_static_method_import(
      std::uint32_t index);
  std::optional<virtual_method_import> get_virtual_method_import(
      std::uint32_t index);
  std::optional<dynamic_method_import> get_dynamic_method_import(
      std::uint32_t index);

  std::optional<classloading::raw_string> load_constant_string(std::uint32_t index);

  std::optional<std::uint32_t> reflect_object_field_index(
      classloading::raw_string str, datatype expected_type);
  std::optional<std::uint32_t> reflect_class_field_index(
      classloading::raw_string str, datatype expected_type);
  std::optional<methods::method> reflect_static_method(classloading::raw_string str);
  std::optional<std::uint32_t> reflect_virtual_method_index(classloading::raw_string str);
  std::optional<methods::method> reflect_dynamic_method(classloading::raw_string str);

  std::optional<methods::method> lookup_virtual_method_direct(
      std::uint32_t idx);

  static std::uintptr_t get_static_memory_idx(std::uintptr_t idx);

  std::uint32_t total_class_size() const;

  std::uint32_t get_self_index() const;

  template <typename out_t>
  std::optional<out_t> checked_read_static_memory(std::uint32_t idx) const {
    if constexpr (std::is_same_v<out_t, base_object>) {
      void *raw = this->class_data.read<void *>(get_static_memory_idx(idx));
      return base_object(raw);
    } else {
      return this->class_data.read<out_t>(get_static_memory_idx(idx));
    }
  }

  template <typename in_t>
  bool checked_write_static_memory(std::uint32_t idx, in_t value) {
    if constexpr (std::is_same_v<in_t, base_object>) {
      this->class_data.write(get_static_memory_idx(idx), value.get_raw());
      return true;
    } else {
      this->class_data.write(get_static_memory_idx(idx), value);
      return true;
    }
  }

  gc::iterable<gc::class_static_pointer_iterator> static_pointers();
  std::size_t virtual_method_count() const;
  std::array<std::uint32_t, 7> inherited_variable_counts() const;
};
}  // namespace classes
}  // namespace oops

#endif