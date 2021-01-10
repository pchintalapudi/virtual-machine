#include "../classes/object.h"
#include "../classloader/classloader.h"
#include "stack_iterators.h"
#include "class_iterators.h"

using namespace oops::memory;
using namespace oops::classloading;
using namespace oops::gc;

stack_frame_iterator stack::begin() {
  return stack_frame_iterator(*this->current);
}

stack_frame_iterator stack::end() {
  return stack_frame_iterator(stack::frame(this->stack_root));
}

frame_pointer_iterator stack::frame::begin() {
  return frame_pointer_iterator(*this, 0);
}

frame_pointer_iterator stack::frame::end() {
  return frame_pointer_iterator(*this,
                                this->executing_method().double_offset());
}

class_iterator classloader::begin() {
    return class_iterator(this->metaspace.root());
}

class_iterator classloader::end() {
    return class_iterator(static_cast<char*>(this->metaspace.root()) + this->metaspace.amount_used());
}