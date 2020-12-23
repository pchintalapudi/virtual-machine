#ifndef OOPS_CLASSES_CLASS_H
#define OOPS_CLASSES_CLASS_H

#include <optional>
#include <variant>

#include "../memory/byteblock.h"
#include "../methods/method.h"
#include "object.h"

namespace oops {
namespace methods {
class method;
}
namespace classes {

class clazz;
class string;
typedef std::variant<string, clazz> class_descriptor;
struct field_descriptor;
struct static_method_descriptor;
struct virtual_method_descriptor;
struct dynamic_method_descriptor;
enum class datatype;

class clazz {
 private:
  memory::byteblock<> class_data;

  std::optional<std::uint32_t> reflect_index(string str);

 public:
  clazz(void *cls);

  void *get_raw() const { return class_data.get_raw(); }

  std::optional<clazz> superclass() const;

  std::uint32_t object_size() const;

  std::optional<field_descriptor> get_field_descriptor(std::uint32_t index);
  std::optional<class_descriptor> get_class_descriptor(std::uint32_t index);
  std::optional<static_method_descriptor> get_static_method_descriptor(
      std::uint32_t index);
  std::optional<virtual_method_descriptor> get_virtual_method_descriptor(
      std::uint32_t index);
  std::optional<dynamic_method_descriptor> get_dynamic_method_descriptor(
      std::uint32_t index);

  std::optional<string> load_constant_string(std::uint32_t index);

  std::optional<std::uint32_t> reflect_object_field_index(
      string str, datatype expected_type);
  std::optional<std::uint32_t> reflect_class_field_index(
      string str, datatype expected_type);
  std::optional<methods::method> reflect_static_method(string str);
  std::optional<std::uint32_t> reflect_virtual_method_index(string str);
  std::optional<methods::method> reflect_dynamic_method(string str);

  std::optional<methods::method> lookup_virtual_method_direct(std::uint32_t idx);

  std::uintptr_t get_static_memory_idx(std::uintptr_t idx) {
    return idx +
           56;  // Size of header region, defined in /spec/ClassStructure.md
  }

  template <typename out_t>
  std::optional<out_t> checked_read_static_memory(std::uint32_t idx) {
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
};
}  // namespace classes
}  // namespace oops

#endif