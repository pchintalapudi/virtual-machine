#include "../classes/class_header.h"
#include "../classes/datatypes.h"
#include "class_file_io.h"
#include "loaded_iterators.h"

using namespace oops::classloading;

#define read_header(field)                                \
  this->cls.read<decltype(classes::class_header::field)>( \
      offsetof(classes::class_header, field))
#define write_header(htype, value)                        \
  this->cls.write(offsetof(classes::class_header, htype), \
                  static_cast<decltype(classes::class_header::htype)>(value))

bool class_writer::initialize(memory::bump_allocator &allocator,
                              std::uintptr_t size) {
  auto allocated = allocator.allocate(size);
  this->allocated = size;
  this->cls.initialize(*allocated);
  return !!allocated;
}
void class_writer::set_superclass(classes::clazz superclass) {
  write_header(superclass, superclass.get_raw());
}
void class_writer::set_static_variable_offsets(
    std::array<std::uint32_t, 7> offsets) {
  write_header(static_byte_offset,
               offsets[static_cast<unsigned>(classes::datatype::BYTE)]);
  write_header(static_short_offset,
               offsets[static_cast<unsigned>(classes::datatype::SHORT)]);
  write_header(static_int_offset,
               offsets[static_cast<unsigned>(classes::datatype::INT)]);
  write_header(static_float_offset,
               offsets[static_cast<unsigned>(classes::datatype::FLOAT)]);
  write_header(static_long_offset,
               offsets[static_cast<unsigned>(classes::datatype::LONG)]);
  write_header(static_double_offset,
               offsets[static_cast<unsigned>(classes::datatype::DOUBLE)]);
  write_header(static_total_size,
               offsets[static_cast<unsigned>(classes::datatype::OBJECT)]);
}
void class_writer::set_vmt_offset(std::uint32_t offset) {
  write_header(vmt_offset, offset);
}
void class_writer::set_class_import_table_offset(std::uint32_t offset) {
  write_header(class_import_offset, offset);
}
void class_writer::set_field_import_table_offset(std::uint32_t offset) {
  write_header(field_import_offset, offset);
}
void class_writer::set_export_table_offset(std::uint32_t offset) {
  write_header(exports_offset, offset);
}
void class_writer::set_method_bytecodes_offset(std::uint32_t offset) {
  write_header(bytecodes_offset, offset);
}
void class_writer::set_string_pool_offset(std::uint32_t offset) {
  write_header(string_pool_offset, offset);
}
void class_writer::set_class_index(std::uint32_t index) {
  write_header(class_index, index);
}
void class_writer::revoke(memory::bump_allocator &allocator) {
  allocator.deallocate(this->allocated);
}
oops::classes::clazz class_writer::commit() {
  return classes::clazz(this->cls.get_raw());
}

void class_writer::bulk_load_strings(
    oops::classloading::class_file_reader &reader) {
  auto read_from = static_cast<const char *>(reader.file.get_raw()) +
                   reader.string_pool_offset();
  auto write_to = static_cast<char *>(this->cls.get_raw()) +
                  read_header(string_pool_offset);
  std::memcpy(write_to, read_from, reader.strings_byte_count());
}

class_iterable<loaded_class_reference_iterator>
class_writer::class_references() {
  return class_iterable(
      loaded_class_reference_iterator(this, read_header(class_import_offset)),
      loaded_class_reference_iterator(this, read_header(field_import_offset)));
}
class_iterable<loaded_import_reference_iterator>
class_writer::import_references() {
  return class_iterable(
      loaded_import_reference_iterator(this, read_header(field_import_offset)),
      loaded_import_reference_iterator(this, read_header(exports_offset)));
}
class_iterable<loaded_field_reference_iterator>
class_writer::field_references() {
  return class_iterable(
      loaded_field_reference_iterator(this, read_header(exports_offset)),
      loaded_field_reference_iterator(this, read_header(bytecodes_offset)));
}

std::uint32_t class_writer::translate_string_index(std::uint32_t base_offset) {
  return base_offset + read_header(string_pool_offset);
}

std::uint32_t class_writer::translate_method_index(std::uint32_t base_offset) {
  return base_offset + read_header(bytecodes_offset);
}

void class_writer::set_virtual_method(std::uint32_t index,
                                      std::uint32_t method_index) {
  void *mtd = static_cast<char *>(this->cls.get_raw()) +
              read_header(bytecodes_offset) + method_index;
  this->cls.write(read_header(vmt_offset) + index * sizeof(void *), mtd);
}

void class_writer::preload_virtual_method_table(classes::clazz superclass) {
  void *vmt =
      static_cast<char *>(this->cls.get_raw()) + read_header(vmt_offset);
  memory::byteblock supdata;
  supdata.initialize(superclass.get_raw());
  auto vmt_offset = supdata.read<decltype(classes::class_header::vmt_offset)>(
      offsetof(classes::class_header, vmt_offset));
  auto vmt_end =
      supdata.read<decltype(classes::class_header::class_import_offset)>(
          offsetof(classes::class_header, class_import_offset));
  void *src = static_cast<char *>(supdata.get_raw()) + vmt_offset;
  std::memcpy(vmt, src, vmt_end - vmt_offset);
}