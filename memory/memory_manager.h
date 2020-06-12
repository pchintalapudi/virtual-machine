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

            maybe<objects::instruction> call_static(objects::method method, std::uint16_t return_offset, objects::instruction invoke_instruction)
            {
                typedef objects::field::field_type type;
                auto caller_frame = this->stack();
                bool success = this->_stack.push_frame(method.stack_size(), method.handle_count(), return_offset, method.rval_type(), method);
                if (!success)
                    return {false, objects::instruction(nullptr)};
                auto callee_frame = this->stack();
                auto limit = method.arg_count();
                if (limit)
                {
                    std::uint64_t caller_offset;
                    objects::method::argument_definition callee_args;
                    for (auto argi = 0u; argi < limit; argi++)
                    {
                        if (argi % 4 == 0)
                            caller_offset = *++invoke_instruction;
                        if (argi % 3 == 0)
                            callee_args = method.read_arg(argi / 3);
                        switch (callee_args.arg_types[argi % 3])
                        {
                        case type::CHAR:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<std::int8_t>(caller_offset >> (argi % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::SHORT:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<std::int16_t>(caller_offset >> (argi % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::INT:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<std::int32_t>(caller_offset >> (argi % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::LONG:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<std::int64_t>(caller_offset >> (argi % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::FLOAT:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<float>(caller_offset >> (argi % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::DOUBLE:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<double>(caller_offset >> (argi % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::OBJECT:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<objects::object>(caller_offset >> (argi % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        default:
                            return {false, invoke_instruction};
                        }
                    }
                }
                callee_frame.set_return_address(++invoke_instruction);
                return {true, method.bytecode_begin()};
            }

            maybe<objects::instruction> call_instance(objects::method method, objects::object source, std::uint16_t return_offset, objects::instruction invoke_instruction)
            {
                typedef objects::field::field_type type;
                auto caller_frame = this->stack();
                bool success = this->_stack.push_frame(method.stack_size(), method.handle_count(), return_offset, method.rval_type(), method);
                if (!success)
                    return {false, objects::instruction(nullptr)};
                auto callee_frame = this->stack();
                auto limit = method.arg_count();
                if (limit)
                {
                    std::uint64_t caller_offset;
                    objects::method::argument_definition callee_args = method.read_arg(0);
                    callee_frame.write<objects::object>(callee_args.arg_offsets[0], source);
                    for (auto argi = 1u; argi < limit; argi++)
                    {
                        if ((argi - 1u) % 4 == 0)
                            caller_offset = *++invoke_instruction;
                        if (argi % 3 == 0)
                            callee_args = method.read_arg(argi / 3);
                        switch (callee_args.arg_types[argi % 3])
                        {
                        case type::CHAR:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<std::int8_t>(caller_offset >> ((argi - 1u) % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::SHORT:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<std::int16_t>(caller_offset >> ((argi - 1u) % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::INT:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<std::int32_t>(caller_offset >> ((argi - 1u) % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::LONG:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<std::int64_t>(caller_offset >> ((argi - 1u) % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::FLOAT:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<float>(caller_offset >> ((argi - 1u) % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::DOUBLE:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<double>(caller_offset >> ((argi - 1u) % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        case type::OBJECT:
                            callee_frame.write(callee_args.arg_offsets[argi % 3], caller_frame.read<objects::object>(caller_offset >> ((argi - 1u) % 4 * sizeof(std::uint16_t) * 8) & 0xffffull));
                            break;
                        default:
                            return {false, invoke_instruction};
                        }
                    }
                }
                callee_frame.set_return_address(++invoke_instruction);
                return {true, method.bytecode_begin()};
            }

            bool new_object(objects::clazz clz)
            {
                return this->_heap.allocate(clz);
            }
        };
    } // namespace memory
} // namespace oops
#endif