#include "class.h"

#include <algorithm>
#include <tuple>

#include "../gc/class_iterators.h"
#include "field_descriptor.h"
#include "object.h"

using namespace oops::classes;

namespace {
namespace header {
#define dumb_type(name, tp) \
  struct name {             \
    tp value;               \
  };
dumb_type(superclass, void *);
dumb_type(static_total_size, std::uint32_t);
dumb_type(static_double_offset, std::uint32_t);
dumb_type(static_long_offset, std::uint32_t);
dumb_type(static_float_offset, std::uint32_t);
dumb_type(static_int_offset, std::uint32_t);
dumb_type(static_short_offset, std::uint32_t);
dumb_type(static_byte_offset, std::uint32_t);
dumb_type(instance_total_size, std::uint32_t);
dumb_type(instance_double_offset, std::uint32_t);
dumb_type(instance_long_offset, std::uint32_t);
dumb_type(instance_float_offset, std::uint32_t);
dumb_type(instance_int_offset, std::uint32_t);
dumb_type(instance_short_offset, std::uint32_t);
dumb_type(instance_byte_offset, std::uint32_t);
dumb_type(vmt_offset, std::uint32_t);
dumb_type(class_descriptor_offset, std::uint32_t);
dumb_type(field_descriptor_offset, std::uint32_t);
dumb_type(reflections_offset, std::uint32_t);
dumb_type(bytecodes_offset, std::uint32_t);
dumb_type(string_pool_offset, std::uint32_t);
dumb_type(class_size, std::uint32_t);
dumb_type(class_index, std::uint32_t);
#undef dumb_type
};  // namespace header

template <typename of, typename T, typename... Args>
constexpr unsigned offset_of() {
  if constexpr (std::is_same_v<T, of>) {
    return 0;
  } else {
    return sizeof(T) + offset_of<of, Args...>();
  }
}
template <typename of, typename T, typename... Args>
constexpr unsigned index_of() {
  if constexpr (std::is_same_v<T, of>) {
    return 0;
  } else {
    return index_of<of, Args...>() + 1;
  }
}

template <typename of, typename... Args>
constexpr unsigned offset_of_v = offset_of<of, Args...>();

template <typename of, typename... Args>
constexpr unsigned index_of_v = index_of<of, Args...>();

#define hto(t) header::t
#define header_types                                                           \
  hto(superclass), hto(static_total_size), hto(static_double_offset),          \
      hto(static_long_offset), hto(static_float_offset),                       \
      hto(static_int_offset), hto(static_short_offset),                        \
      hto(static_byte_offset), hto(instance_total_size),                       \
      hto(instance_double_offset), hto(instance_long_offset),                  \
      hto(instance_float_offset), hto(instance_int_offset),                    \
      hto(instance_short_offset), hto(instance_byte_offset), hto(vmt_offset),  \
      hto(class_descriptor_offset), hto(field_descriptor_offset),              \
      hto(reflections_offset), hto(bytecodes_offset), hto(string_pool_offset), \
      hto(class_size), hto(class_index)
template <typename htype>
using header_type_of = decltype(htype::value);

template <typename htype, typename... Args>
using end_type_of =
    std::tuple_element_t<index_of_v<htype, Args...> + 1, std::tuple<Args...>>;

constexpr unsigned total_header_size =
    offset_of_v<header::class_index, header_types> +
    sizeof(header_type_of<header::class_index>);
}  // namespace

clazz::clazz(void *cls) { class_data.initialize(cls); }

std::optional<oops::classes::clazz> clazz::superclass() const {
  void *superclass = this->class_data.read<header_type_of<header::superclass>>(
      offset_of_v<header::superclass, header_types>);
  if (superclass) {
    return clazz(superclass);
  }
  return {};
}

std::uint32_t clazz::object_size() const {
  return this->class_data.read<header_type_of<header::instance_total_size>>(
      offset_of_v<header::instance_total_size, header_types>);
}

std::uint32_t clazz::static_pointer_offset() const { return 0; }
std::uint32_t clazz::static_double_offset() const {
  return this->class_data.read<header_type_of<header::static_double_offset>>(
      offset_of_v<header::static_double_offset, header_types>);
}
std::uint32_t clazz::static_long_offset() const {
  return this->class_data.read<header_type_of<header::static_long_offset>>(
      offset_of_v<header::static_long_offset, header_types>);
}
std::uint32_t clazz::static_float_offset() const {
  return this->class_data.read<header_type_of<header::static_float_offset>>(
      offset_of_v<header::static_float_offset, header_types>);
}
std::uint32_t clazz::static_int_offset() const {
  return this->class_data.read<header_type_of<header::static_int_offset>>(
      offset_of_v<header::static_int_offset, header_types>);
}
std::uint32_t clazz::static_short_offset() const {
  return this->class_data.read<header_type_of<header::static_short_offset>>(
      offset_of_v<header::static_short_offset, header_types>);
}
std::uint32_t clazz::static_byte_offset() const {
  return this->class_data.read<header_type_of<header::static_byte_offset>>(
      offset_of_v<header::static_byte_offset, header_types>);
}
std::uint32_t clazz::instance_pointer_offset() const { return 0; }
std::uint32_t clazz::instance_double_offset() const {
  return this->class_data.read<header_type_of<header::static_double_offset>>(
      offset_of_v<header::static_double_offset, header_types>);
}
std::uint32_t clazz::instance_long_offset() const {
  return this->class_data.read<header_type_of<header::static_long_offset>>(
      offset_of_v<header::static_long_offset, header_types>);
}
std::uint32_t clazz::instance_float_offset() const {
  return this->class_data.read<header_type_of<header::static_float_offset>>(
      offset_of_v<header::static_float_offset, header_types>);
}
std::uint32_t clazz::instance_int_offset() const {
  return this->class_data.read<header_type_of<header::static_int_offset>>(
      offset_of_v<header::static_int_offset, header_types>);
}
std::uint32_t clazz::instance_short_offset() const {
  return this->class_data.read<header_type_of<header::static_short_offset>>(
      offset_of_v<header::static_short_offset, header_types>);
}
std::uint32_t clazz::instance_byte_offset() const {
  return this->class_data.read<header_type_of<header::static_byte_offset>>(
      offset_of_v<header::static_byte_offset, header_types>);
}

std::optional<field_descriptor> clazz::get_field_descriptor(
    std::uint32_t index) {
  auto field_descriptor_start =
      this->class_data.read<header_type_of<header::field_descriptor_offset>>(
          offset_of_v<header::field_descriptor_offset, header_types>);
  index *= sizeof(std::uint32_t) * 2;
  auto location = field_descriptor_start + index;
  field_descriptor out = {
      .clazz = *this->get_class_descriptor(this->class_data.read<std::uint32_t>(
          location + sizeof(std::uint32_t))),
      .field_index = *this->load_constant_string(
          this->class_data.read<std::uint32_t>(location))};
  return out;
}
std::optional<class_descriptor> clazz::get_class_descriptor(
    std::uint32_t index) {
  auto class_descriptor_start =
      this->class_data.read<header_type_of<header::class_descriptor_offset>>(
          offset_of_v<header::class_descriptor_offset, header_types>);
  index *= sizeof(std::uint32_t);
  auto location = class_descriptor_start + index;
  std::uint32_t string_offset = this->class_data.read<std::uint32_t>(location);
  return *this->load_constant_string(string_offset);
}
std::optional<static_method_descriptor> clazz::get_static_method_descriptor(
    std::uint32_t index) {
  auto field_descriptor_start =
      this->class_data.read<header_type_of<header::field_descriptor_offset>>(
          offset_of_v<header::field_descriptor_offset, header_types>);
  index *= sizeof(std::uint32_t) * 2;
  auto location = field_descriptor_start + index;
  static_method_descriptor out = {
      .clazz = *this->get_class_descriptor(this->class_data.read<std::uint32_t>(
          location + sizeof(std::uint32_t))),
      .static_method = *this->load_constant_string(
          this->class_data.read<std::uint32_t>(location))};
  return out;
}
std::optional<virtual_method_descriptor> clazz::get_virtual_method_descriptor(
    std::uint32_t index) {
  auto field_descriptor_start =
      this->class_data.read<header_type_of<header::field_descriptor_offset>>(
          offset_of_v<header::field_descriptor_offset, header_types>);
  index *= sizeof(std::uint32_t) * 2;
  auto location = field_descriptor_start + index;
  virtual_method_descriptor out = {
      .clazz = *this->get_class_descriptor(this->class_data.read<std::uint32_t>(
          location + sizeof(std::uint32_t))),
      .virtual_method_index = *this->load_constant_string(
          this->class_data.read<std::uint32_t>(location))};
  return out;
}
std::optional<dynamic_method_descriptor> clazz::get_dynamic_method_descriptor(
    std::uint32_t index) {
  auto maybe_name = this->load_constant_string(index);
  if (maybe_name) {
    return (dynamic_method_descriptor){.dynamic_method_name = *maybe_name};
  } else {
    return {};
  }
}

std::optional<string> clazz::load_constant_string(std::uint32_t index) {
  auto string_pool_start =
      this->class_data.read<header_type_of<header::string_pool_offset>>(
          offset_of_v<header::string_pool_offset, header_types>);
  auto string_index = string_pool_start + index * sizeof(void *);
  if (string_index < this->class_data.read<header_type_of<header::class_size>>(
                         offset_of_v<header::class_size, header_types>)) {
    void *str = this->class_data.read<void *>(string_index);
    return string(str);
  }
  return {};
}

std::optional<std::uint32_t> clazz::reflect_object_field_index(string str,
                                                               datatype) {
  return this->reflect_index(str);
}
std::optional<std::uint32_t> clazz::reflect_class_field_index(string str,
                                                              datatype) {
  return this->reflect_index(str);
}
std::optional<oops::methods::method> clazz::reflect_static_method(string str) {
  auto idx = this->reflect_index(str);
  if (idx) {
    auto start =
        this->class_data.read<header_type_of<header::bytecodes_offset>>(
            offset_of_v<header::bytecodes_offset, header_types>);
    char *raw = static_cast<char *>(this->get_raw()) + start +
                *idx * sizeof(std::uint64_t);
    return methods::method(raw);
  }
  return {};
}
std::optional<std::uint32_t> clazz::reflect_virtual_method_index(string str) {
  return this->reflect_index(str);
}
std::optional<oops::methods::method> clazz::reflect_dynamic_method(string str) {
  auto idx = this->reflect_index(str);
  if (idx) {
    auto start =
        this->class_data.read<header_type_of<header::bytecodes_offset>>(
            offset_of_v<header::bytecodes_offset, header_types>);
    char *raw = static_cast<char *>(this->get_raw()) + start +
                *idx * sizeof(std::uint64_t);
    return methods::method(raw);
  }
  return {};
}

std::optional<oops::methods::method> clazz::lookup_virtual_method_direct(
    std::uint32_t idx) {
  auto vmt_start = this->class_data.read<header_type_of<header::vmt_offset>>(
      offset_of_v<header::vmt_offset, header_types>);
  auto real = vmt_start + idx * sizeof(void *);
  return methods::method(this->class_data.read<void *>(real));
}

namespace {
class reflection_table_iterator {
 private:
  oops::memory::byteblock<> table;
  std::uint32_t index;

 public:
  typedef std::uint32_t reference;
  typedef std::uint32_t value_type;
  typedef std::uint32_t difference_type;
  typedef void pointer;
  typedef std::random_access_iterator_tag iterator_category;
  reflection_table_iterator(void *table, std::uint32_t index) {
    this->table.initialize(table);
    this->index = index;
  }
  reference operator*() const {
    return table.read<std::uint32_t>(index * sizeof(std::uint32_t) * 2);
  }
  reflection_table_iterator &operator++() {
    ++this->index;
    return *this;
  }
  reflection_table_iterator operator++(int) {
    reflection_table_iterator out = *this;
    ++*this;
    return out;
  }
  reflection_table_iterator &operator--() {
    --this->index;
    return *this;
  }
  reflection_table_iterator operator--(int) {
    reflection_table_iterator out = *this;
    --*this;
    return out;
  }
  reflection_table_iterator &operator+=(difference_type n) {
    this->index += n;
    return *this;
  }
  reflection_table_iterator &operator-=(difference_type n) {
    this->index -= n;
    return *this;
  }
  reflection_table_iterator operator+(difference_type n) const {
    auto copy = *this;
    return copy += n;
  }
  friend reflection_table_iterator operator+(
      difference_type n, const reflection_table_iterator &it) {
    return it + n;
  }
  reflection_table_iterator operator-(difference_type n) const {
    auto copy = *this;
    return copy -= n;
  }
  difference_type operator-(const reflection_table_iterator &other) const {
    return this->index - other.index;
  }
  reference operator[](difference_type n) {
    return table.read<std::uint32_t>((this->index + n) * sizeof(std::uint32_t) *
                                     2);
  }
  bool operator==(const reflection_table_iterator &other) const {
    return this->index == other.index;
  }
  bool operator!=(const reflection_table_iterator &other) const {
    return this->index != other.index;
  }
  bool operator<(const reflection_table_iterator &other) const {
    return this->index < other.index;
  }
  bool operator<=(const reflection_table_iterator &other) const {
    return this->index <= other.index;
  }
  bool operator>(const reflection_table_iterator &other) const {
    return this->index > other.index;
  }
  bool operator>=(const reflection_table_iterator &other) const {
    return this->index >= other.index;
  }
  std::uint32_t offset() const {
    return this->table.read<std::uint32_t>(this->index + sizeof(std::uint32_t));
  }
};
}  // namespace

std::optional<std::uint32_t> clazz::reflect_index(string str) {
  auto reflection_table_idx =
      this->class_data.read<header_type_of<header::reflections_offset>>(
          offset_of_v<header::reflections_offset, header_types>);
  char *byteblock_begin =
      static_cast<char *>(this->class_data.get_raw()) + reflection_table_idx;
  auto end_table_idx = this->class_data.read<
      header_type_of<end_type_of<header::reflections_offset, header_types>>>(
      offset_of_v<end_type_of<header::reflections_offset, header_types>,
                  header_types>);
  std::uint32_t diff =
      (end_table_idx - reflection_table_idx) / (sizeof(std::uint32_t) * 2);
  reflection_table_iterator begin(byteblock_begin, 0),
      end(byteblock_begin, diff);
  auto lt = [this](std::uint32_t i1, string str2) {
    auto str1 = *this->load_constant_string(i1);
    std::int32_t l1 = str1.length(), l2 = str2.length();
    int value = std::memcmp(str1.to_char_array(), str2.to_char_array(),
                            std::min(l1, l2)) < 0;
    return value < 0 || (value == 0 && l1 < l2);
  };
  auto bound = std::lower_bound(begin, end, str, lt);
  if (bound != end) {
    auto cmp = *this->load_constant_string(*bound);
    if (cmp.length() == str.length() &&
        memcmp(cmp.to_char_array(), str.to_char_array(), cmp.length()) == 0) {
      return bound.offset();
    }
  }
  return {};
}

std::uintptr_t clazz::get_static_memory_idx(std::uintptr_t idx) {
  return idx + total_header_size;
}

std::uint32_t clazz::get_self_index() const {
  return this->class_data.read<header_type_of<header::class_index>>(
      offset_of_v<header::class_index, header_types>);
}

oops::gc::iterable<oops::gc::class_static_pointer_iterator>
clazz::static_pointers() {
  auto pointer_start = total_header_size;
  auto pointer_end =
      this->class_data.read<header_type_of<header::static_double_offset>>(
          offset_of_v<header::static_double_offset, header_types>);
  auto end_offset = pointer_end - pointer_start;
  return gc::iterable(gc::class_static_pointer_iterator(*this, 0),
                      gc::class_static_pointer_iterator(*this, end_offset));
}
oops::gc::iterable<oops::gc::class_string_iterator> clazz::strings() {
  auto strings_start =
      this->class_data.read<header_type_of<header::string_pool_offset>>(
          offset_of_v<header::string_pool_offset, header_types>);
  auto strings_end = this->class_data.read<header_type_of<header::class_size>>(
      offset_of_v<header::class_size, header_types>);
  auto string_count = (strings_end - strings_start) / sizeof(void *);
  return gc::iterable(gc::class_string_iterator(*this, 0),
                      gc::class_string_iterator(*this, string_count));
}
void clazz::replace_string(std::uint32_t idx, base_object obj) {
  auto start =
      this->class_data.read<header_type_of<header::string_pool_offset>>(
          offset_of_v<header::string_pool_offset, header_types>);
  idx *= sizeof(void *);
  this->class_data.write(start + idx, obj.get_raw());
}

std::uint32_t clazz::total_class_size() const {
  return this->class_data.read<header_type_of<header::class_size>>(
      offset_of_v<header::class_size, header_types>);
}