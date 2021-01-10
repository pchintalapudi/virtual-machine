#include "oheap.h"

#include "../core/executor.h"
#include "../gc/class_iterators.h"
#include "../gc/stop_and_copy.h"

using namespace oops::memory;

std::uintptr_t aligned_memory_amount(std::uintptr_t amount) {
  return (amount + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
}

std::optional<oops::classes::object> heap::allocate_object(classes::clazz cls) {
  auto maybe_memory = this->allocate_memory(cls.object_size() + sizeof(void *));
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
void heap::register_executor(core::executor *executor) {
  this->vm_executors.insert(executor);
}
void heap::unregister_executor(core::executor *executor) {
  this->vm_executors.erase(executor);
}
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
  for (auto executor : this->vm_executors) {
    destination =
        gc::track_stack_pointers(&executor->vm_stack, destination, low, high);
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

std::optional<oops::classes::base_object> heap::reify_constant_string(
    classloading::raw_string str) {
  auto to_allocate = str.length + sizeof(std::uint32_t) * 2;
  // TODO codify this in a specification somewhere
  // Strings only have an array pointer, a 32-bit hashcode (algorithm TBD), and
  // a boolean (byte) indicating whether the hashcode is valid. Net size is 13
  // bytes of pure object + 8 bytes of class pointer = 21 bytes allocated size =
  // 24 padded bytes
  to_allocate += 24;
  auto block = this->allocate_memory(to_allocate);
  if (!block) {
    return {};
  }
  auto string = classes::string(*block);
  byteblock sclass;
  sclass.initialize(*block);
  sclass.write(0, this->bootstrap_classloader.string_class().get_raw());
  char *array = static_cast<char *>(*block) + 24;
  string.write(0, array);
  std::memcpy(array + sizeof(std::uint32_t) * 2, str.string, str.length);
  classes::array(array).initialize(str.length, classes::datatype::BYTE);
  return string.to_base_object();
}