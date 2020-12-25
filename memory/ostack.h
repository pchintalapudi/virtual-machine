#ifndef OOPS_MEMORY_OSTACK_H
#define OOPS_MEMORY_OSTACK_H

#include <optional>

#include "../classes/class.h"
#include "../classes/object.h"
#include "../globals/types.h"
#include "../methods/method.h"
#include "../native/native_types.h"
#include "byteblock.h"

namespace oops {
namespace gc {
class stack_frame_iterator;
class frame_pointer_iterator;
}  // namespace gc
namespace memory {
class stack {
 public:
  class frame {
   private:
    byteblock<> mem;

    static std::uintptr_t frame_header_offset();

    template <typename out_t>
    out_t read(stack_idx_t offset) const {
      return mem.read<out_t>(static_cast<std::uintptr_t>(offset) *
                                 sizeof(std::int32_t) +
                             frame_header_offset());
    }

    template <typename in_t>
    void write(stack_idx_t offset, in_t value) {
      mem.write<in_t>(
          static_cast<std::uintptr_t>(offset) * sizeof(std::int32_t) +
              frame_header_offset(),
          value);
    }

    friend class stack;

    void set_return_offset(stack_idx_t offset);
    void set_return_address(instr_idx_t address);
    void set_previous_frame(frame prev);
    void set_context_class(classes::clazz context);
    void set_executing_method(methods::method executing);

   public:
    frame(void *mem) { this->mem.initialize(mem); }

    void *get_raw() const { return this->mem.get_raw(); }

    template <typename out_t>
    std::optional<out_t> checked_read(stack_idx_t offset) const {
      if constexpr (std::is_same_v<classes::base_object, out_t>) {
        void *pointer = this->read<void *>(offset);
        return classes::base_object(pointer);
      } else {
        return this->read<out_t>(offset);  // TODO actually check the type
      }
    }

    template <typename in_t>
    bool checked_write(stack_idx_t offset, in_t value) {
      if constexpr (std::is_same_v<classes::base_object, in_t>) {
        void *pointer = value.get_raw();
        this->write(offset, pointer);
        return true;
      } else {
        this->write(offset, value);  // TODO actually check the type
        return true;
      }
    }

    stack::frame previous_frame() const;
    classes::clazz context_class() const;
    methods::method executing_method() const;

    stack_idx_t get_return_offset() const;
    instr_idx_t get_return_address() const;

    std::uint32_t total_size() const;

    gc::frame_pointer_iterator begin();
    gc::frame_pointer_iterator end();
  };

 private:
  frame current;
  void *stack_root;
  std::uintptr_t max_stack_size;

  bool advance_frame(std::uint32_t allocated_stack);

 public:
  bool initialize(std::uintptr_t max_stack_size);

  frame &current_frame() { return this->current; }

  bool try_push_frame(classes::clazz context, methods::method method,
                      methods::args args, stack_idx_t return_offset,
                      instr_idx_t return_address);
  bool try_push_native_frame(classes::clazz context, methods::method method,
                             const oops_wrapper_t *args, std::uint8_t nargs);
  void pop_frame();

  void destroy();

  gc::stack_frame_iterator begin();
  gc::stack_frame_iterator end();
};
}  // namespace memory
}  // namespace oops

#endif