#include "class.h"

#include <algorithm>
#include <tuple>

#include "../gc/class_iterators.h"
#include "class_header.h"
#include "field_import.h"
#include "object.h"

using namespace oops::classes;

clazz::clazz(void *cls) { class_data.initialize(cls); }

#define read_header(field)                              \
  this->class_data.read<decltype(class_header::field)>( \
      offsetof(class_header, field))

std::optional<oops::classes::clazz> clazz::superclass() const {
  void *superclass = read_header(superclass);
  if (superclass) {
    return clazz(superclass);
  }
  return {};
}

std::uint32_t clazz::object_size() const {
  return read_header(instance_total_size);
}

std::uint32_t clazz::static_pointer_offset() const { return 0; }
std::uint32_t clazz::static_double_offset() const {
  return read_header(static_double_offset);
}
std::uint32_t clazz::static_long_offset() const {
  return read_header(static_long_offset);
}
std::uint32_t clazz::static_float_offset() const {
  return read_header(static_float_offset);
}
std::uint32_t clazz::static_int_offset() const {
  return read_header(static_int_offset);
}
std::uint32_t clazz::static_short_offset() const {
  return read_header(static_short_offset);
}
std::uint32_t clazz::static_byte_offset() const {
  return read_header(static_byte_offset);
}
std::uint32_t clazz::instance_pointer_offset() const { return 0; }
std::uint32_t clazz::instance_double_offset() const {
  return read_header(instance_double_offset);
}
std::uint32_t clazz::instance_long_offset() const {
  return read_header(instance_long_offset);
}
std::uint32_t clazz::instance_float_offset() const {
  return read_header(instance_float_offset);
}
std::uint32_t clazz::instance_int_offset() const {
  return read_header(instance_int_offset);
}
std::uint32_t clazz::instance_short_offset() const {
  return read_header(instance_short_offset);
}
std::uint32_t clazz::instance_byte_offset() const {
  return read_header(instance_byte_offset);
}

std::optional<field_import> clazz::get_field_import(std::uint32_t index) {
  auto field_import_start = read_header(field_import_offset);
  index *= sizeof(std::uint32_t) * 2;
  auto location = field_import_start + index;
  field_import out = {
      .clazz = *this->get_class_import(this->class_data.read<std::uint32_t>(
          location + sizeof(std::uint32_t))),
      .field_index = *this->load_constant_string(
          this->class_data.read<std::uint32_t>(location))};
  return out;
}
std::optional<class_import> clazz::get_class_import(std::uint32_t index) {
  auto class_import_start = read_header(class_import_offset);
  index *= sizeof(std::uint32_t);
  auto location = class_import_start + index;
  std::uint32_t string_offset = this->class_data.read<std::uint32_t>(location);
  return *this->load_constant_string(string_offset);
}
std::optional<static_method_import> clazz::get_static_method_import(
    std::uint32_t index) {
  auto field_import_start = read_header(field_import_offset);
  index *= sizeof(std::uint32_t) * 2;
  auto location = field_import_start + index;
  static_method_import out = {
      .clazz = *this->get_class_import(this->class_data.read<std::uint32_t>(
          location + sizeof(std::uint32_t))),
      .static_method = *this->load_constant_string(
          this->class_data.read<std::uint32_t>(location))};
  return out;
}
std::optional<virtual_method_import> clazz::get_virtual_method_import(
    std::uint32_t index) {
  auto field_import_start = read_header(field_import_offset);
  index *= sizeof(std::uint32_t) * 2;
  auto location = field_import_start + index;
  virtual_method_import out = {
      .clazz = *this->get_class_import(this->class_data.read<std::uint32_t>(
          location + sizeof(std::uint32_t))),
      .virtual_method_index = *this->load_constant_string(
          this->class_data.read<std::uint32_t>(location))};
  return out;
}
std::optional<dynamic_method_import> clazz::get_dynamic_method_import(
    std::uint32_t index) {
  auto maybe_name = this->load_constant_string(index);
  if (maybe_name) {
    return (dynamic_method_import){.dynamic_method_name = *maybe_name};
  } else {
    return {};
  }
}

std::optional<oops::classloading::raw_string> clazz::load_constant_string(
    std::uint32_t index) {
  auto string_pool_start = read_header(string_pool_offset);
  auto string_index = string_pool_start + index * sizeof(void *);
  if (string_index < read_header(class_size)) {
    void *str = this->class_data.read<void *>(string_index);
    std::int32_t length;
    memcpy(&length, static_cast<char *>(str) - sizeof(std::int32_t),
           sizeof(std::int32_t));
    return (classloading::raw_string){.string = static_cast<const char *>(str),
                                      .length = length};
  }
  return {};
}

std::optional<std::uint32_t> clazz::reflect_object_field_index(
    classloading::raw_string str, datatype) {
  return this->reflect_index(str);
}
std::optional<std::uint32_t> clazz::reflect_class_field_index(
    classloading::raw_string str, datatype) {
  return this->reflect_index(str);
}
std::optional<oops::methods::method> clazz::reflect_static_method(
    classloading::raw_string str) {
  auto idx = this->reflect_index(str);
  if (idx) {
    auto start = read_header(bytecodes_offset);
    char *raw = static_cast<char *>(this->get_raw()) + start +
                *idx * sizeof(std::uint64_t);
    return methods::method(raw);
  }
  return {};
}
std::optional<std::uint32_t> clazz::reflect_virtual_method_index(
    classloading::raw_string str) {
  auto maybe = this->reflect_index_recursively(str);
  return maybe ? maybe->second : std::optional<std::uint32_t>();
}
std::optional<oops::methods::method> clazz::reflect_dynamic_method(
    classloading::raw_string str) {
  auto idx = this->reflect_virtual_method_index(str);
  if (idx) {
    return this->lookup_virtual_method_direct(*idx);
  }
  return {};
}

std::optional<oops::methods::method> clazz::lookup_virtual_method_direct(
    std::uint32_t idx) {
  auto vmt_start = read_header(vmt_offset);
  auto real = vmt_start + idx * sizeof(void *);
  return methods::method(this->class_data.read<void *>(real));
}

namespace {
class export_table_iterator {
 private:
  oops::memory::byteblock<> table;
  std::uint32_t index;

 public:
  typedef std::uint32_t reference;
  typedef std::uint32_t value_type;
  typedef std::uint32_t difference_type;
  typedef void pointer;
  typedef std::random_access_iterator_tag iterator_category;
  export_table_iterator(void *table, std::uint32_t index) {
    this->table.initialize(table);
    this->index = index;
  }
  reference operator*() const {
    return table.read<std::uint32_t>(index * sizeof(std::uint32_t) * 2);
  }
  export_table_iterator &operator++() {
    ++this->index;
    return *this;
  }
  export_table_iterator operator++(int) {
    export_table_iterator out = *this;
    ++*this;
    return out;
  }
  export_table_iterator &operator--() {
    --this->index;
    return *this;
  }
  export_table_iterator operator--(int) {
    export_table_iterator out = *this;
    --*this;
    return out;
  }
  export_table_iterator &operator+=(difference_type n) {
    this->index += n;
    return *this;
  }
  export_table_iterator &operator-=(difference_type n) {
    this->index -= n;
    return *this;
  }
  export_table_iterator operator+(difference_type n) const {
    auto copy = *this;
    return copy += n;
  }
  friend export_table_iterator operator+(difference_type n,
                                         const export_table_iterator &it) {
    return it + n;
  }
  export_table_iterator operator-(difference_type n) const {
    auto copy = *this;
    return copy -= n;
  }
  difference_type operator-(const export_table_iterator &other) const {
    return this->index - other.index;
  }
  reference operator[](difference_type n) {
    return table.read<std::uint32_t>((this->index + n) * sizeof(std::uint32_t) *
                                     2);
  }
  bool operator==(const export_table_iterator &other) const {
    return this->index == other.index;
  }
  bool operator!=(const export_table_iterator &other) const {
    return this->index != other.index;
  }
  bool operator<(const export_table_iterator &other) const {
    return this->index < other.index;
  }
  bool operator<=(const export_table_iterator &other) const {
    return this->index <= other.index;
  }
  bool operator>(const export_table_iterator &other) const {
    return this->index > other.index;
  }
  bool operator>=(const export_table_iterator &other) const {
    return this->index >= other.index;
  }
  std::uint32_t offset() const {
    return this->table.read<std::uint32_t>(this->index + sizeof(std::uint32_t));
  }
};
}  // namespace

std::optional<std::uint32_t> clazz::reflect_index(
    classloading::raw_string str) {
  auto export_table_idx = read_header(exports_offset);
  char *byteblock_begin =
      static_cast<char *>(this->class_data.get_raw()) + export_table_idx;
  auto end_table_idx = read_header(bytecodes_offset);
  std::uint32_t diff =
      (end_table_idx - export_table_idx) / (sizeof(std::uint32_t) * 2);
  export_table_iterator begin(byteblock_begin, 0), end(byteblock_begin, diff);
  auto lt = [this](std::uint32_t i1, classloading::raw_string str2) {
    auto str1 = *this->load_constant_string(i1);
    std::int32_t l1 = str1.length, l2 = str2.length;
    int value = std::memcmp(str1.string, str2.string, std::min(l1, l2)) < 0;
    return value < 0 || (value == 0 && l1 < l2);
  };
  auto bound = std::lower_bound(begin, end, str, lt);
  if (bound != end) {
    auto cmp = *this->load_constant_string(*bound);
    if (cmp.length == str.length &&
        memcmp(cmp.string, str.string, cmp.length) == 0) {
      return bound.offset() & 0x00'FF'FF'FF;
    }
  }
  return {};
}

std::optional<std::pair<clazz, std::uint32_t>> clazz::reflect_index_recursively(
    classloading::raw_string str) {
  auto this_class_found = this->reflect_index(str);
  if (this_class_found) {
    return std::pair{*this, *this_class_found};
  }
  auto super = this->superclass();
  if (super) {
    return super->reflect_index_recursively(str);
  }
  return {};
}

std::uintptr_t clazz::get_static_memory_idx(std::uintptr_t idx) {
  return idx + sizeof(class_header);
}

std::uint32_t clazz::get_self_index() const { return read_header(class_index); }

oops::gc::iterable<oops::gc::class_static_pointer_iterator>
clazz::static_pointers() {
  auto pointer_start = sizeof(class_header);
  auto pointer_end = read_header(static_double_offset);
  auto end_offset = pointer_end - pointer_start;
  return gc::iterable(gc::class_static_pointer_iterator(*this, 0),
                      gc::class_static_pointer_iterator(*this, end_offset));
}

std::uint32_t clazz::total_class_size() const {
  return read_header(class_size);
}

std::uint32_t clazz::get_instance_offset(std::uint32_t internal_offset,
                                         datatype dt) {
  switch (dt) {
    case datatype::BYTE:
      return this->instance_byte_offset() + internal_offset * datatype_size(dt);
    case datatype::SHORT:
      return this->instance_short_offset() +
             internal_offset * datatype_size(dt);
    case datatype::INT:
      return this->instance_int_offset() + internal_offset * datatype_size(dt);
    case datatype::FLOAT:
      return this->instance_float_offset() +
             internal_offset * datatype_size(dt);
    case datatype::LONG:
      return this->instance_long_offset() + internal_offset * datatype_size(dt);
    case datatype::DOUBLE:
      return this->instance_double_offset() +
             internal_offset * datatype_size(dt);
    case datatype::OBJECT:
      return this->instance_pointer_offset() +
             internal_offset * datatype_size(dt);
  }
}

std::size_t clazz::virtual_method_count() const {
  return (read_header(class_import_offset) - read_header(vmt_offset)) /
         sizeof(void *);
}

std::array<std::uint32_t, 7> clazz::inherited_variable_counts() const {
  std::array<std::uint32_t, 7> counts{};
  counts[static_cast<unsigned>(datatype::OBJECT)] =
      (this->instance_double_offset() - this->instance_pointer_offset()) /
      datatype_size(datatype::OBJECT);
  counts[static_cast<unsigned>(datatype::DOUBLE)] =
      (this->instance_long_offset() - this->instance_double_offset()) /
      datatype_size(datatype::DOUBLE);
  counts[static_cast<unsigned>(datatype::LONG)] =
      (this->instance_float_offset() - this->instance_long_offset()) /
      datatype_size(datatype::LONG);
  counts[static_cast<unsigned>(datatype::FLOAT)] =
      (this->instance_int_offset() - this->instance_float_offset()) /
      datatype_size(datatype::FLOAT);
  counts[static_cast<unsigned>(datatype::INT)] =
      (this->instance_short_offset() - this->instance_int_offset()) /
      datatype_size(datatype::INT);
  counts[static_cast<unsigned>(datatype::SHORT)] =
      (this->instance_byte_offset() - this->instance_short_offset()) /
      datatype_size(datatype::SHORT);
  counts[static_cast<unsigned>(datatype::BYTE)] =
      (this->object_size() - this->instance_byte_offset()) /
      datatype_size(datatype::BYTE);
  return counts;
}