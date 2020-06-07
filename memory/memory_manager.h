#ifndef MEMORY_MEMORY_MANAGER_H
#define MEMORY_MEMORY_MANAGER_H

#include "stack.h"
#include "heap.h"

namespace oops
{
    namespace memory
    {

        struct mm_args
        {
            memory::stack_args stack_args;
            memory::heap_args heap_args;
        };

        class memory_manager
        {
        private:
            heap _heap;
            stack _stack;

            void gc() {}

        public:
            bool init(const mm_args &args)
            {
                return this->_heap.init(args.heap_args) and this->_stack.init(args.stack_args);
            }
            auto stack()
            {
                return this->_stack.current_frame();
            }

            void invoke()
            {
                //TODO
            }

            bool new_object(objects::clazz clz)
            {
                return this->_heap.allocate(clz);
            }
        };
    } // namespace memory
} // namespace oops
#endif