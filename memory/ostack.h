#ifndef OOPS_MEMORY_OSTACK_H
#define OOPS_MEMORY_OSTACK_H

#include <optional>

#include "../globals/types.h"

#include "byteblock.h"
#include "../methods/method.h"
#include "../classes/object.h"
#include "../classes/class.h"

namespace oops {
    namespace memory {
        class stack {
        public:
            class frame {
                private:

                byteblock<> mem;
                classes::clazz context;
                methods::method executing;

                void initialize(void* mem) {
                    this->mem.initialize(mem);
                }

                template<typename out_t>
                out_t read(stack_idx_t offset) const {
                    return mem.read<out_t>(static_cast<std::uintptr_t>(offset) * sizeof(std::int32_t));
                }

                template<typename in_t>
                void write(stack_idx_t offset, in_t value) {
                    mem.write<in_t>(static_cast<std::uintptr_t>(offset) * sizeof(std::int32_t), value);
                }

                public:

                template<typename out_t>
                std::optional<out_t> checked_read(stack_idx_t offset) {
                    if constexpr (std::is_same_v<classes::base_object, out_t>) {
                        void* pointer = this->read<void*>(offset);
                        return classes::base_object(pointer);
                    } else {
                        return this->read<out_t>(offset); // TODO actually check the type
                    }
                }

                template<typename in_t>
                bool checked_write(stack_idx_t offset, in_t value) {
                    if constexpr (std::is_same_v<classes::base_object, in_t>) {
                        void* pointer = value.get_raw();
                        this->write(offset, pointer);
                        return true;
                    } else {
                        this->write(offset, value); // TODO actually check the type
                        return true;
                    }
                }

                classes::clazz context_class() const {
                    return this->context;
                }
                methods::method executing_method() const {
                    return this->executing;
                }
            };

        private:
        frame current;

        public:
        frame& current_frame();

        void push_frame(methods::method method);
        };
    }
}

#endif