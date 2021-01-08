#include "class_file_io.h"
#include "loader_iterators.h"

using namespace oops::classloading;
namespace {

struct class_file_header {
  std::uint32_t magic_number;
  std::uint16_t implemented_count;
  std::uint16_t method_count;
  std::uint32_t instance_field_table_offset;
  std::uint32_t static_field_table_offset;
  std::uint32_t import_table_offset;
  std::uint32_t method_table_offset;
  std::uint32_t string_pool_offset;
};
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
  return sizeof(class_file_header);
}

#define read_header(field)                             \
  this->file.read<decltype(class_file_header::field)>( \
      offsetof(class_file_header, field))

std::uint32_t class_file_reader::instance_field_table_offset() const {
  return read_header(instance_field_table_offset);
}
std::uint32_t class_file_reader::static_field_table_offset() const {
  return read_header(static_field_table_offset);
}
std::uint32_t class_file_reader::import_table_offset() const {
  return read_header(import_table_offset);
}
std::uint32_t class_file_reader::method_table_offset() const {
  return read_header(method_table_offset);
}
std::uint32_t class_file_reader::string_pool_offset() const {
  return read_header(string_pool_offset);
}

std::uint32_t class_file_reader::magic_number() const {
  return read_header(magic_number);
}
std::uint16_t class_file_reader::implemented_count() const {
  return read_header(implemented_count);
}
std::size_t class_file_reader::defined_method_count() const {
  return read_header(method_count);
}
std::uint32_t class_file_reader::total_class_file_size() const {
  return this->backing->file_size();
}
class_iterable<class_reference_iterator> class_file_reader::class_references() {
  return class_iterable(
      class_reference_iterator(this, sizeof(class_file_header)),
      class_reference_iterator(this, read_header(instance_field_table_offset)));
}
class_iterable<instance_field_reference_iterator>
class_file_reader::instance_field_references() {
  return class_iterable(instance_field_reference_iterator(
                            this, read_header(instance_field_table_offset)),
                        instance_field_reference_iterator(
                            this, read_header(static_field_table_offset)));
}
class_iterable<static_field_reference_iterator>
class_file_reader::static_field_references() {
  return class_iterable(
      static_field_reference_iterator(this,
                                      read_header(static_field_table_offset)),
      static_field_reference_iterator(this, read_header(import_table_offset)));
}
class_iterable<import_iterator> class_file_reader::imports() {
  return class_iterable(
      import_iterator(this, read_header(import_table_offset)),
      import_iterator(this, read_header(method_table_offset)));
}
class_iterable<method_iterator> class_file_reader::methods() {
  return class_iterable(method_iterator(this, read_header(method_table_offset)),
                        method_iterator(this, read_header(string_pool_offset)));
}

std::size_t class_file_reader::class_reference_count() const {
  return (this->instance_field_table_offset() -
          this->class_reference_table_offset()) /
         sizeof(std::uint32_t);
}
std::size_t class_file_reader::import_count() const {
  return (this->method_table_offset() - this->import_table_offset()) /
         sizeof(std::uint32_t);
}
std::size_t class_file_reader::instance_field_count() const {
  return (this->static_field_table_offset() -
          this->instance_field_table_offset()) /
         sizeof(std::uint32_t);
}
std::size_t class_file_reader::static_field_count() const {
  return (this->import_table_offset() - this->static_field_table_offset()) /
         sizeof(std::uint32_t);
}
std::size_t class_file_reader::methods_size() const {
  return this->string_pool_offset() - this->method_table_offset();
}
std::size_t class_file_reader::strings_byte_count() const {
  return this->total_class_file_size() - this->string_pool_offset();
}