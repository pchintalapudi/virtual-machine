#ifndef OOPS_MEMORY_OSTACK_H
#define OOPS_MEMORY_OSTACK_H

#include <optional>

#include "../globals/types.h"

#include "byteblock.h"
#include "../methods/method.h"

namespace oops {
    namespace memory {
        class stack {
        public:
            class frame {
                private:

                friend class stack;

                byteblock<> mem;

                void initialize(void* mem) {
                    this->mem.initialize(mem);
                }

                template<typename out_t>
                std::enable_if_t<std::is_trivially_copy_constructible_v<out_t> and std::is_trivially_constructible_v<out_t>, out_t> read(stack_idx_t offset) const {
                    return mem.read<out_t>(static_cast<std::uintptr_t>(offset) * sizeof(std::int32_t));
                }

                template<typename in_t>
                std::enable_if_t<std::is_trivially_copy_constructible_v<in_t> and std::is_trivially_constructible_v<in_t>, void> write(stack_idx_t offset, in_t value) {
                    mem.write<in_t>(static_cast<std::uintptr_t>(offset) * sizeof(std::int32_t), value);
                }

                public:

                template<typename out_t>
                std::optional<out_t> checked_read(stack_idx_t offset) {
                    return this->read<out_t>(offset); // TODO actually check the type
                }

                template<typename in_t>
                bool checked_write(stack_idx_t offset, in_t value) {
                    this->write(offset, value); // TODO actually check the type
                    return true;
                }
            };

        private:
        public:
        frame& current_frame();

        void push_frame(methods::method* method);
        };
    }
}

#endif