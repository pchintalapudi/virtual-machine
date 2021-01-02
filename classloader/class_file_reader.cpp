#include "class_file_io.h"
#include "loader_iterators.h"

using namespace oops::classloading;
namespace {
namespace header {
#define dumb_type(name, tp) \
  struct name {             \
    tp value;               \
  };
dumb_type(magic_number, std::uint16_t);
dumb_type(implemented_count, std::uint16_t);
dumb_type(instance_field_table_offset, std::uint32_t);
dumb_type(static_field_table_offset, std::uint32_t);
dumb_type(method_table_offset, std::uint32_t);
dumb_type(string_pool_offset, std::uint32_t);
}  // namespace header

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
  hto(magic_number), hto(implemented_count), hto(instance_field_table_offset), \
      hto(static_field_table_offset), hto(method_table_offset),                \
      hto(string_pool_offset)
template <typename htype>
using header_type_of = decltype(htype::value);

template <typename htype, typename... Args>
using end_type_of =
    std::tuple_element_t<index_of_v<htype, Args...> + 1, std::tuple<Args...>>;

constexpr unsigned total_header_size =
    offset_of_v<header::string_pool_offset, header_types> +
    sizeof(header_type_of<header::string_pool_offset>);
}  // namespace

bool class_file_reader::initialize(const char *filename, std::int32_t length) {
  this->backing = platform::mmap_file::create(filename, length);
  if (this->backing) {
    this->file.initialize(**this->backing);
    return true;
  }
  return false;
}
void class_file_reader::destroy() { this->backing.reset(); }

std::uint32_t class_file_reader::class_reference_table_offset() const {
  return total_header_size;
}
std::uint32_t class_file_reader::instance_field_table_offset() const {
  return this->file.read<header_type_of<header::instance_field_table_offset>>(
      offset_of_v<header::instance_field_table_offset, header_types>);
}
std::uint32_t class_file_reader::static_field_table_offset() const {
  return this->file.read<header_type_of<header::static_field_table_offset>>(
      offset_of_v<header::static_field_table_offset, header_types>);
}
std::uint32_t class_file_reader::method_table_offset() const {
  return this->file.read<header_type_of<header::method_table_offset>>(
      offset_of_v<header::method_table_offset, header_types>);
}
std::uint32_t class_file_reader::string_pool_offset() const {
  return this->file.read<header_type_of<header::string_pool_offset>>(
      offset_of_v<header::string_pool_offset, header_types>);
}

std::uint16_t class_file_reader::magic_number() const {
  return this->file.read<header_type_of<header::magic_number>>(
      offset_of_v<header::magic_number, header_types>);
}
std::uint16_t class_file_reader::implemented_count() const {
  return this->file.read<header_type_of<header::implemented_count>>(
      offset_of_v<header::implemented_count, header_types>);
}
std::uint32_t class_file_reader::total_class_file_size() const {
  return this->backing->file_size();
}
class_iterable<class_reference_iterator> class_file_reader::class_references() {
  return class_iterable(
      class_reference_iterator(this, total_header_size),
      class_reference_iterator(
          this,
          this->file.read<header_type_of<header::instance_field_table_offset>>(
              offset_of_v<header::instance_field_table_offset, header_types>)));
}
// class_iterable<class_reference_iterator>
// class_file_reader::instance_field_references();
// class_iterable<class_reference_iterator>
// class_file_reader::static_field_references();
// class_iterable<class_reference_iterator> class_file_reader::methods();