#include "ostack.h"

#include <algorithm>
#include <array>

using namespace oops::memory;

namespace {

struct stack_frame_header {
  void *previous_frame_pointer;
  void *context_class;
  const void *executing_method;
  std::uint16_t return_address;
  std::uint16_t return_offset;
  std::uint32_t total_size;
};
}  // namespace

std::uintptr_t stack::frame::frame_header_offset() {
  return sizeof(stack_frame_header);
}

#define read_header(field)                             \
  this->mem.read<decltype(stack_frame_header::field)>( \
      offsetof(stack_frame_header, field))

oops::classes::clazz stack::frame::context_class() const {
  return classes::clazz(read_header(context_class));
}
oops::methods::method stack::frame::executing_method() const {
  return methods::method(read_header(executing_method));
}

oops::stack_idx_t stack::frame::get_return_offset() const {
  return read_header(return_offset);
}
oops::instr_idx_t stack::frame::get_return_address() const {
  return read_header(return_address);
}

std::uint32_t stack::frame::total_size() const {
  return read_header(total_size);
}

stack::frame stack::frame::previous_frame() const {
  return stack::frame(read_header(previous_frame_pointer));
}

#define write_header(field, value)                     \
  this->mem.write(offsetof(stack_frame_header, field), \
                  static_cast<decltype(stack_frame_header::field)>(value))

void stack::frame::set_return_offset(stack_idx_t offset) {
  write_header(return_offset, offset);
}
void stack::frame::set_return_address(instr_idx_t address) {
  write_header(return_address, address);
}
void stack::frame::set_previous_frame(frame prev) {
  write_header(previous_frame_pointer, prev.get_raw());
}
void stack::frame::set_context_class(classes::clazz context) {
  write_header(context_class, context.get_raw());
}
void stack::frame::set_executing_method(methods::method executing) {
  write_header(executing_method, executing.get_raw());
}

void stack::pop_frame() { this->current = this->current->previous_frame(); }

bool stack::try_push_frame(classes::clazz context, methods::method method,
                           methods::args args, stack_idx_t return_offset,
                           instr_idx_t return_address) {
  frame previous = *this->current;
  methods::method prev_method = previous.executing_method();
  if (!this->advance_frame(method.stack_frame_size() * sizeof(std::int32_t) +
                           sizeof(stack_frame_header))) {
    return false;
  }
  auto bounds = prev_method.get_bounds();
  auto counts = bounds;
  auto types = method.get_arg_types();
  for (int i = 0; i < args.length(); i++) {
    switch (types[i]) {
#define copy_arg(dt, lower, tp)                                           \
  case methods::arg_types::type::dt: {                                    \
    this->current->checked_write(                                          \
        counts[static_cast<int>(methods::arg_types::type::dt)],           \
        *previous.checked_read<tp>(args[i]));                             \
    counts[static_cast<int>(methods::arg_types::type::dt)] += sizeof(tp); \
    break;                                                                \
  }
      copy_arg(INT, int, std::int32_t);
      copy_arg(FLOAT, float, float);
      copy_arg(LONG, long, std::int64_t);
      copy_arg(DOUBLE, double, double);
      copy_arg(OBJECT, pointer, classes::base_object);
#undef copy_arg
    }
  }
  for (unsigned i = 0; i < method.double_offset() * sizeof(std::int32_t);
       i += sizeof(void *)) {
    this->current->checked_write(i, classes::base_object(nullptr));
  }
  this->current->set_return_offset(return_offset);
  this->current->set_return_address(return_address);
  this->current->set_previous_frame(previous);
  this->current->set_context_class(context);
  this->current->set_executing_method(method);
  return true;
}
bool stack::try_push_native_frame(classes::clazz context,
                                  methods::method method,
                                  const oops_wrapper_t *args,
                                  std::uint8_t nargs) {
  frame previous = *this->current;
  if (!this->advance_frame(method.stack_frame_size() * sizeof(std::int32_t) +
                           sizeof(stack_frame_header))) {
    return false;
  }
  auto counts = method.get_bounds();
  auto types = method.get_arg_types();
  for (int i = 0; i < nargs; i++) {
    switch (types[i]) {
#define copy_arg(dt, lower, tp)                                           \
  case methods::arg_types::type::dt: {                                    \
    this->current->checked_write(                                          \
        counts[static_cast<int>(methods::arg_types::type::dt)],           \
        args[i].as_##lower);                                              \
    counts[static_cast<int>(methods::arg_types::type::dt)] += sizeof(tp); \
    break;                                                                \
  }
      copy_arg(INT, int, std::int32_t);
      copy_arg(FLOAT, float, float);
      copy_arg(LONG, long, std::int64_t);
      copy_arg(DOUBLE, double, double);
      case methods::arg_types::type::OBJECT: {
        this->current->checked_write(
            counts[static_cast<int>(methods::arg_types::type::OBJECT)],
            classes::base_object(args[i].as_object->object));
        counts[static_cast<int>(methods::arg_types::type::OBJECT)] +=
            sizeof(void *);
        break;
      }
    }
  }
  for (unsigned i = 0; i < method.double_offset() * sizeof(std::int32_t);
       i += sizeof(void *)) {
    this->current->checked_write(i, classes::base_object(nullptr));
  }
  this->current->set_return_offset(0);
  this->current->set_return_address(0);
  this->current->set_previous_frame(previous);
  this->current->set_context_class(context);
  this->current->set_executing_method(method);
  return true;
}

bool stack::advance_frame(std::uint32_t total_size) {
  auto start = static_cast<char *>(this->current->mem.get_raw());
  auto end = this->current->total_size() + start;
  if (end + total_size <=
      this->max_stack_size + static_cast<char *>(this->stack_root)) {
    frame next{end};
    next.mem.write(offsetof(stack_frame_header, total_size), total_size);
    this->current = next;
    return true;
  }
  return false;
}