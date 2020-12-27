#include "ostack.h"

#include <algorithm>
#include <array>

using namespace oops::memory;

namespace {
namespace header {
#define dumb_type(name, tp) \
  struct name {             \
    tp value;               \
  };
dumb_type(previous_frame_pointer, void *);
dumb_type(context_class, void *);
dumb_type(executing_method, void *);
dumb_type(return_address, std::uint16_t);
dumb_type(return_offset, std::uint16_t);
dumb_type(total_size, std::uint32_t);
}  // namespace header

template <typename of, typename T, typename... Args>
constexpr unsigned offset_of() {
  if constexpr (std::is_same_v<T, of>) {
    return 0;
  } else {
    return sizeof(T) + offset_of<of, Args...>();
  }
}
template <typename of, typename T, typename... Args>
constexpr unsigned index_of() {
  if constexpr (std::is_same_v<T, of>) {
    return 0;
  } else {
    return index_of<of, Args...>() + 1;
  }
}

template <typename of, typename... Args>
constexpr unsigned offset_of_v = offset_of<of, Args...>();

template <typename of, typename... Args>
constexpr unsigned index_of_v = index_of<of, Args...>();

#define hto(t) header::t
#define header_types                                                      \
  hto(previous_frame_pointer), hto(context_class), hto(executing_method), \
      hto(return_address), hto(return_offset), hto(total_size)
template <typename htype>
using header_type_of = decltype(htype::value);

template <typename htype, typename... Args>
using end_type_of =
    std::tuple_element_t<index_of_v<htype, Args...> + 1, std::tuple<Args...>>;

constexpr unsigned total_header_size =
    offset_of_v<header::total_size, header_types> +
    sizeof(header_type_of<header::total_size>);
}  // namespace

std::uintptr_t stack::frame::frame_header_offset() { return total_header_size; }

oops::classes::clazz stack::frame::context_class() const {
  return classes::clazz(this->mem.read<header_type_of<header::context_class>>(
      offset_of_v<header::context_class, header_types>));
}
oops::methods::method stack::frame::executing_method() const {
  return methods::method(
      this->mem.read<header_type_of<header::executing_method>>(
          offset_of_v<header::executing_method, header_types>));
}

oops::stack_idx_t stack::frame::get_return_offset() const {
  return this->mem.read<header_type_of<header::return_offset>>(
      offset_of_v<header::return_offset, header_types>);
}
oops::instr_idx_t stack::frame::get_return_address() const {
  return this->mem.read<header_type_of<header::return_address>>(
      offset_of_v<header::return_address, header_types>);
}

std::uint32_t stack::frame::total_size() const {
  return this->mem.read<header_type_of<header::total_size>>(
      offset_of_v<header::return_address, header_types>);
}

stack::frame stack::frame::previous_frame() const {
  return stack::frame(
      this->mem.read<header_type_of<header::previous_frame_pointer>>(
          offset_of_v<header::previous_frame_pointer, header_types>));
}

void stack::frame::set_return_offset(stack_idx_t offset) {
  this->mem.write(offset_of_v<header::return_offset, header_types>, offset);
}
void stack::frame::set_return_address(instr_idx_t address) {
  this->mem.write(offset_of_v<header::return_address, header_types>, address);
}
void stack::frame::set_previous_frame(frame prev) {
  this->mem.write(offset_of_v<header::previous_frame_pointer, header_types>,
                  prev.mem.get_raw());
}
void stack::frame::set_context_class(classes::clazz context) {
  this->mem.write(offset_of_v<header::context_class, header_types>,
                  context.get_raw());
}
void stack::frame::set_executing_method(methods::method executing) {
  this->mem.write(offset_of_v<header::executing_method, header_types>,
                  executing.get_raw());
}

void stack::pop_frame() { this->current = this->current.previous_frame(); }

bool stack::try_push_frame(classes::clazz context, methods::method method,
                           methods::args args, stack_idx_t return_offset,
                           instr_idx_t return_address) {
  frame previous = this->current;
  methods::method prev_method = previous.executing_method();
  if (!this->advance_frame(method.stack_frame_size() * sizeof(std::int32_t) +
                           total_header_size)) {
    return false;
  }
  auto bounds = prev_method.get_bounds();
  auto counts = bounds;
  auto types = method.get_arg_types();
  for (int i = 0; i < args.length(); i++) {
    switch (types[i]) {
#define copy_arg(dt, lower, tp)                                           \
  case methods::arg_types::type::dt: {                                    \
    this->current.checked_write(                                          \
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
    this->current.checked_write(i, classes::base_object(nullptr));
  }
  this->current.set_return_offset(return_offset);
  this->current.set_return_address(return_address);
  this->current.set_previous_frame(previous);
  this->current.set_context_class(context);
  this->current.set_executing_method(method);
  return true;
}
bool stack::try_push_native_frame(classes::clazz context,
                                  methods::method method,
                                  const oops_wrapper_t *args,
                                  std::uint8_t nargs) {
  frame previous = this->current;
  if (!this->advance_frame(method.stack_frame_size() * sizeof(std::int32_t) +
                           total_header_size)) {
    return false;
  }
  auto counts = method.get_bounds();
  auto types = method.get_arg_types();
  for (int i = 0; i < nargs; i++) {
    switch (types[i]) {
#define copy_arg(dt, lower, tp)                                           \
  case methods::arg_types::type::dt: {                                    \
    this->current.checked_write(                                          \
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
        this->current.checked_write(
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
    this->current.checked_write(i, classes::base_object(nullptr));
  }
  this->current.set_return_offset(0);
  this->current.set_return_address(0);
  this->current.set_previous_frame(previous);
  this->current.set_context_class(context);
  this->current.set_executing_method(method);
  return true;
}

bool stack::advance_frame(std::uint32_t total_size) {
  auto start = static_cast<char *>(this->current.mem.get_raw());
  auto end = this->current.total_size() + start;
  if (end + total_size <=
      this->max_stack_size + static_cast<char *>(this->stack_root)) {
    frame next{end};
    next.mem.write(offset_of_v<header::total_size, header_types>, total_size);
    this->current = next;
    return true;
  }
  return false;
}