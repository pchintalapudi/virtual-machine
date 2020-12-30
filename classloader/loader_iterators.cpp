#include "loader_iterators.h"

using namespace oops::classloading;

raw_string class_reference_iterator::operator*() {
    this->reader;
}

class_reference_iterator &class_reference_iterator::operator++() {
  this->index++;
  return *this;
}
class_reference_iterator class_reference_iterator::operator+(
    std::uint32_t n) const {
  auto other = *this;
  other.index += n;
  return other;
}
bool class_reference_iterator::operator==(
    const class_reference_iterator &other) const {
  return this->reader->file == other.reader->file && this->index == other.index;
}