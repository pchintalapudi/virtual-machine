#include "loader_iterators.h"

#include <climits>

#include "../methods/method_header.h"

using namespace oops::classloading;

class_reference class_reference_iterator::operator*() {
  std::uintptr_t offset = this->backing->class_reference_table_offset();
  offset += this->index * sizeof(std::uint32_t);
  auto string_offset = this->backing->file.read<std::uint32_t>(offset);
  return class_reference{.name = this->backing->load_raw_string(string_offset),
                         .string_idx = string_offset};
}

import_reference import_iterator::operator*() {
  std::uintptr_t metadata_offset = this->backing->import_table_offset();
  metadata_offset += this->index * sizeof(std::uint32_t) * 2;
  auto class_reference_offset = metadata_offset + sizeof(std::uint16_t);
  auto string_reference_offset = class_reference_offset + sizeof(std::uint16_t);
  auto metadata = this->backing->file.read<std::uint16_t>(metadata_offset);
  auto class_reference =
      this->backing->file.read<std::uint16_t>(class_reference_offset);
  auto string_offset =
      this->backing->file.read<std::uint32_t>(string_reference_offset);
  return import_reference{
      .type = static_cast<classes::field_type>(metadata >> CHAR_BIT),
      .subtype = static_cast<std::uint8_t>(metadata & 0xff),
      .name = this->backing->load_raw_string(string_offset),
      .string_idx = string_offset,
      .class_reference = class_reference};
}

field static_field_reference_iterator::operator*() {
  std::uintptr_t offset = this->backing->static_field_table_offset();
  offset += this->index + sizeof(std::uint32_t);
  auto data = this->backing->file.read<std::uint32_t>(offset);
  return field{.name = this->backing->load_raw_string(data & 0xFF'FF'FF),
               .string_idx = data & 0x00'FF'FF'FF,
               .data_idx = 0,
               .field_type = classes::field_type::STATIC,
               .data_type = static_cast<std::uint8_t>(data >> CHAR_BIT * 3)};
}

field instance_field_reference_iterator::operator*() {
  std::uintptr_t offset = this->backing->instance_field_table_offset();
  offset += this->index + sizeof(std::uint32_t);
  auto data = this->backing->file.read<std::uint32_t>(offset);
  return field{.name = this->backing->load_raw_string(data & 0xFF'FF'FF),
               .string_idx = data & 0xFF'FF'FF,
               .data_idx = 0,
               .field_type = classes::field_type::INSTANCE,
               .data_type = static_cast<std::uint8_t>(data >> CHAR_BIT * 3)};
}

bool field::operator<(const field &other) const {
  if (other.name.string == name.string) return false;
  int result = std::memcmp(other.name.string, name.string,
                           std::min(other.name.length, name.length));
  return result < 0 || (result == 0 && name.length < other.name.length);
}

method_iterator &method_iterator::operator++() {
  auto mtd = methods::method(
      static_cast<const char *>(this->reader->file.get_raw()) + this->index);
  this->index += mtd.get_total_method_size();
  return *this;
}

field method_iterator::operator*() {
  memory::byteblock<false> method_reader;
  method_reader.initialize(
      static_cast<const char *>(this->reader->file.get_raw()) + this->index);
  auto string_idx = method_reader.read<decltype(methods::method_header::name)>(
      offsetof(methods::method_header, name));
  auto metadata = string_idx >> CHAR_BIT * 3;
  string_idx &= 0x00'FF'FF'FF;
  return field{.name = this->reader->load_raw_string(string_idx),
               .string_idx = string_idx,
               .data_idx = static_cast<std::uint32_t>(this->index - this->reader->method_table_offset()),
               .field_type = classes::field_type::METHOD,
               .data_type = static_cast<std::uint8_t>(metadata)};
}