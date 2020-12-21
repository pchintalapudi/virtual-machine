#include "oheap.h"

using namespace oops::memory;

std::optional<oops::classes::object> heap::allocate_object(classes::clazz cls) {
  auto maybe_memory = this->allocate_memory(cls.object_size());
  if (!maybe_memory) {
    return {};
  }
  return classes::object(*maybe_memory);
}

std::optional<oops::classes::array> heap::allocate_array(classes::datatype dt,
                                                         std::int32_t length) {
  std::uintptr_t to_allocate_bytes = length;
  to_allocate_bytes *= classes::datatype_size(dt);
  to_allocate_bytes += sizeof(std::uint32_t) * 2;
  auto maybe_memory = this->allocate_memory(to_allocate_bytes);
  if (!maybe_memory) {
    return {};
  }
  return classes::array(*maybe_memory);
}