#ifndef OOPS_GC_STACK_ITERATORS_H
#define OOPS_GC_STACK_ITERATORS_H

#include "../memory/ostack.h"

namespace oops {
namespace gc {
class stack_frame_iterator {
 private:
  memory::stack::frame current;

 public:
 stack_frame_iterator(memory::stack::frame current) : current(current) {}
  memory::stack::frame &operator*() { return this->current; }
  stack_frame_iterator &operator++() {
    this->current = this->current.previous_frame();
    return *this;
  }
  stack_frame_iterator operator++(int) {
    auto self = *this;
    ++self;
    return self;
  }
  bool operator==(const stack_frame_iterator &other) const {
    return this->current.get_raw() == other.current.get_raw();
  }
  bool operator!=(const stack_frame_iterator &other) const {
    return this->current.get_raw() != other.current.get_raw();
  }
};
class frame_pointer_iterator {
 private:
  memory::stack::frame frame;
  unsigned pointer;

 public:
  class object_thunk {
   private:
    memory::stack::frame frame;
    unsigned pointer;

   public:
    object_thunk(memory::stack::frame frame, unsigned pointer)
        : frame(frame), pointer(pointer) {}

    operator classes::base_object() const {
      return *frame.checked_read<classes::base_object>(this->pointer);
    }

    object_thunk &operator=(classes::base_object obj) {
      frame.checked_write(this->pointer, obj);
      return *this;
    }
  };

  frame_pointer_iterator(memory::stack::frame frame, unsigned idx) : frame(frame), pointer(idx) {}
  auto operator*() { return object_thunk(frame, pointer); }
  frame_pointer_iterator &operator++() {
    this->pointer += sizeof(void *);
    return *this;
  }
  frame_pointer_iterator operator++(int) {
    auto self = *this;
    ++self;
    return self;
  }
  bool operator==(const frame_pointer_iterator &other) const {
    return this->frame.get_raw() == other.frame.get_raw() &&
           this->pointer == other.pointer;
  }
  bool operator!=(const frame_pointer_iterator &other) const {
    return this->frame.get_raw() != other.frame.get_raw() ||
           this->pointer != other.pointer;
  }
};
}  // namespace gc
}  // namespace oops

#endif