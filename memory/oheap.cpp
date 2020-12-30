#include "oheap.h"

#include "../gc/class_iterators.h"
#include "../gc/stop_and_copy.h"

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
void heap::register_stack(stack *stack) { this->vm_stacks.insert(stack); }
void heap::unregister_stack(stack *stack) { this->vm_stacks.erase(stack); }
oops_object_t *heap::allocate_native_reference(classes::base_object obj) {
  auto &ref = this->native_references[obj.get_raw()];
  if (!ref.second++) {
    ref.first = new oops_object_t{obj.get_raw()};
  }
  return ref.first;
}
void heap::deallocate_native_reference(oops_object_t *obj) {
  auto it = this->native_references.find(obj->object);
  if (it != this->native_references.end()) {
    if (!--it->second.second) {
      delete it->second.first;
      this->native_references.erase(it);
    }
  }
}
std::optional<void *> heap::allocate_memory(std::uintptr_t amount) {
  amount += sizeof(std::uint64_t);
  // Optimistically avoid gc
  auto space = this->allocator.allocate(amount);
  if (space) {
    return static_cast<char *>(*space) + sizeof(std::uint64_t);
  }
  auto dest = this->allocator.gc_prologue();
  void *low = this->allocator.low_bound(), *high = this->allocator.high_bound();
  // Can't even guarantee the gc will work
  if (!dest) {
    return {};
  }
  auto destination = *dest, begin = destination;
  for (auto cls : this->bootstrap_classloader) {
    destination = gc::track_class_pointers(cls, destination, low, high);
  }
  for (auto stack : this->vm_stacks) {
    destination = gc::track_stack_pointers(stack, destination, low, high);
  }
  destination = gc::track_native_pointers(
      this->native_references, this->scratch, destination, low, high);
  destination = gc::cleanup_migrated_objects(begin, destination, low, high);
  this->allocator.gc_epilogue(destination);
  // Post-gc retry
  space = this->allocator.allocate(amount);
  if (space) {
    return static_cast<char *>(*space) + sizeof(std::uint64_t);
  }
  return {};
}