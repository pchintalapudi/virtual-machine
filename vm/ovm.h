#ifndef VM_OVM_H
#define VM_OVM_H

#include "../memory/allocator.h"
#include "stack.h"
#include <tuple>
namespace oops
{
    namespace vm
    {
        struct vm_args
        {
            memory::mm_args heap_args;
            stack::stack_args stack_args;
        };
        class virtual_machine
        {
        private:
            //TODO handler stack
            //TODO handling exception
            oops::stack::stack stack;
            oops::memory::memory_manager heap;
            typedef objects::field::field_type ftype;
            std::vector<char *> next_instruction;

            int invalid_bytecode(std::uint64_t)
            {
                //TODO
                return -1;
            }

            int execute();

        public:
            virtual_machine(vm_args args) : stack(args.stack_args), heap(args.heap_args) {}

        private: //Templating
#pragma region
            template <typename after>
            inline bool read_stack_primitive(after consumer, std::uint16_t offset, objects::field::field_type tp)
            {
                switch (tp)
                {
                case ftype::OBJECT:
                case ftype::METHOD:
                    break;
                case ftype::CHAR:
                    return consumer(this->stack.read<std::int8_t>(offset));
                case ftype::SHORT:
                    return consumer(this->stack.read<std::int16_t>(offset));
                case ftype::INT:
                    return consumer(this->stack.read<std::int32_t>(offset));
                case ftype::FLOAT:
                    return consumer(this->stack.read<float>(offset));
                case ftype::LONG:
                    return consumer(this->stack.read<std::int64_t>(offset));
                case ftype::DOUBLE:
                    return consumer(this->stack.read<double>(offset));
                }
                return false;
            }
            template <typename after>
            inline bool read_stack_integer(after consumer, std::uint16_t offset, objects::field::field_type tp)
            {
                switch (tp)
                {
                case ftype::OBJECT:
                case ftype::METHOD:
                case ftype::DOUBLE:
                case ftype::FLOAT:
                    break;
                case ftype::CHAR:
                    return consumer(this->stack.read<std::int8_t>(offset));
                case ftype::SHORT:
                    return consumer(this->stack.read<std::int16_t>(offset));
                case ftype::INT:
                    return consumer(this->stack.read<std::int32_t>(offset));
                case ftype::LONG:
                    return consumer(this->stack.read<std::int64_t>(offset));
                }
                return false;
            }
            template <typename after>
            inline bool read_imm_primitive(after consumer, std::uint16_t offset, objects::field::field_type tp)
            {
                switch (tp)
                {
                case ftype::OBJECT:
                case ftype::METHOD:
                    break;
                case ftype::CHAR:
                    return consumer(static_cast<std::int8_t>(offset));
                case ftype::SHORT:
                    return consumer(static_cast<std::int16_t>(offset));
                case ftype::INT:
                    return consumer(static_cast<std::int32_t>(offset));
                case ftype::FLOAT:
                    return consumer(static_cast<float>(offset));
                case ftype::LONG:
                    return consumer(static_cast<std::int64_t>(offset));
                case ftype::DOUBLE:
                    return consumer(static_cast<double>(offset));
                }
                return false;
            }
            template <typename after>
            inline bool read_imm_integer(after consumer, std::uint16_t offset, objects::field::field_type tp)
            {
                switch (tp)
                {
                case ftype::OBJECT:
                case ftype::METHOD:
                case ftype::DOUBLE:
                case ftype::FLOAT:
                    break;
                case ftype::CHAR:
                    return consumer(static_cast<std::int8_t>(offset));
                case ftype::SHORT:
                    return consumer(static_cast<std::int16_t>(offset));
                case ftype::INT:
                    return consumer(static_cast<std::int32_t>(offset));
                case ftype::LONG:
                    return consumer(static_cast<std::int64_t>(offset));
                }
                return false;
            }

            template<typename after>
            inline bool read_stack(after consumer, std::uint16_t offset, objects::field::field_type tp) {
                switch (tp)
                {
                case ftype::OBJECT:
                case ftype::METHOD:
                    return consumer(this->stack.read<char*>(offset));
                case ftype::CHAR:
                    return consumer(this->stack.read<std::int8_t>(offset));
                case ftype::SHORT:
                    return consumer(this->stack.read<std::int16_t>(offset));
                case ftype::INT:
                    return consumer(this->stack.read<std::int32_t>(offset));
                case ftype::FLOAT:
                    return consumer(this->stack.read<float>(offset));
                case ftype::LONG:
                    return consumer(this->stack.read<std::int64_t>(offset));
                case ftype::DOUBLE:
                    return consumer(this->stack.read<double>(offset));
                }
                return false;
            }
#pragma endregion
        private: //Primitive ops
#pragma region
            template <typename op, typename param1, typename param2>
            inline bool primitive_execute_writeback(op operation, param1 arg1, param2 arg2, std::uint16_t dest, objects::field::field_type dest_type)
            {
                auto result = operation(arg1, arg2);
                switch (dest_type)
                {
                case ftype::OBJECT:
                case ftype::METHOD:
                    return false;
                case ftype::CHAR:
                    this->stack.write<std::int8_t>(dest, result);
                    break;
                case ftype::SHORT:
                    this->stack.write<std::int16_t>(dest, result);
                    break;
                case ftype::INT:
                    this->stack.write<std::int32_t>(dest, result);
                    break;
                case ftype::FLOAT:
                    this->stack.write<float>(dest, result);
                    break;
                case ftype::LONG:
                    this->stack.write<std::int64_t>(dest, result);
                    break;
                case ftype::DOUBLE:
                    this->stack.write<double>(dest, result);
                    break;
                }
                return true;
            }
#pragma endregion
        private: //Integer ops
#pragma region
            template <typename op, typename param1, typename param2>
            inline bool integer_execute_writeback(op operation, param1 arg1, param2 arg2, std::uint16_t dest, objects::field::field_type dest_type)
            {
                auto result = operation(arg1, arg2);
                switch (dest_type)
                {
                case ftype::OBJECT:
                case ftype::METHOD:
                case ftype::DOUBLE:
                case ftype::FLOAT:
                    return false;
                case ftype::CHAR:
                    this->stack.write<std::int8_t>(dest, result);
                    break;
                case ftype::SHORT:
                    this->stack.write<std::int16_t>(dest, result);
                    break;
                case ftype::INT:
                    this->stack.write<std::int32_t>(dest, result);
                    break;
                case ftype::LONG:
                    this->stack.write<std::int64_t>(dest, result);
                    break;
                }
                return true;
            }
#pragma endregion
        private: //Branching
#pragma region
            bool jump(std::uint16_t offset, signed char forward)
            {
                this->next_instruction.back() += (static_cast<std::int32_t>(offset) << 3 ^ -static_cast<std::int32_t>(forward ^ 1)) + static_cast<std::int32_t>(forward ^ 1);
                return true;
            }

            template <typename op, typename param1, typename param2>
            inline bool branch(op operation, param1 arg1, param2 arg2, std::uint16_t offset, signed char forward)
            {
                return (operation(arg1, arg2) && this->jump(offset, forward)) || true;
            }

#pragma endregion
private:
        };
    } // namespace vm
} // namespace oops
#endif