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
namespace memory {
class stack {
 public:
  class frame {
   private:
    byteblock<> mem;
    classes::clazz context;
    methods::method executing;

    static constexpr std::uintptr_t frame_header_offset = 0;

    template <typename out_t>
    out_t read(stack_idx_t offset) const {
      return mem.read<out_t>(static_cast<std::uintptr_t>(offset) *
                                 sizeof(std::int32_t) +
                             frame_header_offset);
    }

    template <typename in_t>
    void write(stack_idx_t offset, in_t value) {
      mem.write<in_t>(
          static_cast<std::uintptr_t>(offset) * sizeof(std::int32_t) +
              frame_header_offset,
          value);
    }

   public:
    void initialize(void *mem) { this->mem.initialize(mem); }

    stack_idx_t get_return_offset() const;
    instr_idx_t get_return_address() const;

    template <typename out_t>
    std::optional<out_t> checked_read(stack_idx_t offset) {
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

    classes::clazz context_class() const { return this->context; }
    methods::method executing_method() const { return this->executing; }
  };

 private:
  frame current;

 public:
  frame &current_frame();

  void push_frame(classes::clazz context, methods::method method, methods::args args, stack_idx_t return_offset, instr_idx_t return_address);
  void push_native_frame(classes::clazz context, methods::method method, const oops_wrapper_t *args, std::uint8_t nargs);
  void pop_frame();
};
}  // namespace memory
}  // namespace oops

#endif