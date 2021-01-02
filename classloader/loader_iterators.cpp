#include "loader_iterators.h"

using namespace oops::classloading;

class_reference_iterator &class_reference_iterator::operator++() {
  this->index += sizeof(std::uint32_t);
  return *this;
}
class_reference_iterator class_reference_iterator::operator+(
    std::uint32_t n) const {
  auto other = *this;
  other.index += n * sizeof(std::uint32_t);
  return other;
}
bool class_reference_iterator::operator==(
    const class_reference_iterator &other) const {
  return this->reader->file == other.reader->file && this->index == other.index;
}