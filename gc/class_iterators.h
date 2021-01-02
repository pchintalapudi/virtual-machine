#ifndef OOPS_GC_CLASS_ITERATORS_H
#define OOPS_GC_CLASS_ITERATORS_H

#include "../classes/class.h"

namespace oops {
namespace gc {
class class_static_pointer_iterator {
 private:
  classes::clazz subject;
  std::uint32_t index;

 public:
  class_static_pointer_iterator(classes::clazz subject, std::uint32_t index)
      : subject(subject), index(index) {}
  class object_thunk {
   private:
    classes::clazz cls;
    std::uint32_t pointer;

   public:
    object_thunk(classes::clazz cls, std::uint32_t pointer)
        : cls(cls), pointer(pointer) {}

    operator classes::base_object() const {
      return *cls.checked_read_static_memory<classes::base_object>(
          this->pointer);
    }

    object_thunk &operator=(classes::base_object obj) {
      cls.checked_write_static_memory(this->pointer, obj);
      return *this;
    }
  };
  object_thunk operator*() { return object_thunk(this->subject, this->index); }
  auto &operator++() {
    this->index += sizeof(void *);
    return *this;
  }
  auto operator++(int) {
    auto self = *this;
    ++*this;
    return self;
  }
  bool operator==(const class_static_pointer_iterator &other) const {
    return this->subject.get_raw() == other.subject.get_raw() &&
           this->index == other.index;
  }
  bool operator!=(const class_static_pointer_iterator &other) const {
    return this->subject.get_raw() != other.subject.get_raw() ||
           this->index != other.index;
  }
};
class class_iterator {
 private:
  void *cls;

 public:
  class_iterator(void *cls) : cls(cls) {}
  classes::clazz operator*() { return classes::clazz(cls); }
  auto &operator++() {
    this->cls = static_cast<char *>(cls) + (**this).total_class_size();
    return *this;
  }
  auto operator++(int) {
    auto self = *this;
    ++*this;
    return self;
  }
  bool operator==(const class_iterator &other) const {
    return this->cls == other.cls;
  }
  bool operator!=(const class_iterator &other) const {
    return this->cls != other.cls;
  }
};
}  // namespace gc
}  // namespace oops

#endif