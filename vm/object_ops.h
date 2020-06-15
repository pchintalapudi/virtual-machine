#ifndef VM_OBJECT_OPS_H
#define VM_OBJECT_OPS_H

#include "../memory/memory_manager.h"

namespace oops
{
    namespace vm
    {
        struct objects
        {
            template <typename pointer>
            static std::enable_if_t<std::is_same<pointer, oops::objects::object>::value, void> write_object(oops::objects::object dest, std::uint32_t offset, pointer write, memory::memory_manager &manager)
            {
                manager.write_barrier(dest, write);
                dest.write(offset, write);
            }
            template <typename primitive>
            static std::enable_if_t<std::is_signed<primitive>::value, void> write_object(oops::objects::object dest, std::uint32_t offset, primitive write, memory::memory_manager &)
            {
                dest.write(offset, write);
            }
        };
    } // namespace vm
} // namespace oops

#endif