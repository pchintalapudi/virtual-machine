#ifndef VM_OVM_H
#define VM_OVM_H

#include "../memory/memory_manager.h"
#include <tuple>
#include <vector>
namespace oops
{
    namespace vm
    {
        struct vm_args
        {
            memory::mm_args memory_args;
        };
        class virtual_machine
        {
        private:
            std::vector<char *> next_instruction;
            memory::memory_manager memory_manager;

            int invalid_bytecode(std::uint64_t)
            {
                //TODO
                return -1;
            }

            int execute();

        public:
            bool init(const vm_args &args) {
                return memory_manager.init(args.memory_args);
            }

        private: //Reads
#pragma region
            template <typename after>
            inline bool read_stack_primitive(after consumer, std::uint16_t offset, objects::field::field_type tp)
            {
                switch (tp)
                {
                default:
                    break;
                case objects::field::field_type::CHAR:
                    return consumer(this->memory_manager.stack().read<std::int8_t>(offset));
                case objects::field::field_type::SHORT:
                    return consumer(this->memory_manager.stack().read<std::int16_t>(offset));
                case objects::field::field_type::INT:
                    return consumer(this->memory_manager.stack().read<std::int32_t>(offset));
                case objects::field::field_type::FLOAT:
                    return consumer(this->memory_manager.stack().read<float>(offset));
                case objects::field::field_type::LONG:
                    return consumer(this->memory_manager.stack().read<std::int64_t>(offset));
                case objects::field::field_type::DOUBLE:
                    return consumer(this->memory_manager.stack().read<double>(offset));
                }
                return false;
            }
            template <typename after>
            inline bool read_stack_integer(after consumer, std::uint16_t offset, objects::field::field_type tp)
            {
                switch (tp)
                {
                default:
                    break;
                case objects::field::field_type::CHAR:
                    return consumer(this->memory_manager.stack().read<std::int8_t>(offset));
                case objects::field::field_type::SHORT:
                    return consumer(this->memory_manager.stack().read<std::int16_t>(offset));
                case objects::field::field_type::INT:
                    return consumer(this->memory_manager.stack().read<std::int32_t>(offset));
                case objects::field::field_type::LONG:
                    return consumer(this->memory_manager.stack().read<std::int64_t>(offset));
                }
                return false;
            }

            template <typename after>
            inline bool read_imm_primitive(after consumer, std::uint16_t offset, objects::field::field_type tp) {
                return this->read_imm_primitive_32(consumer, offset, tp);
            }

            template <typename after>
            inline bool read_imm_primitive_32(after consumer, std::uint32_t offset, objects::field::field_type tp)
            {
                switch (tp)
                {
                default:
                    break;
                case objects::field::field_type::CHAR:
                    return consumer(static_cast<std::int8_t>(offset));
                case objects::field::field_type::SHORT:
                    return consumer(static_cast<std::int16_t>(offset));
                case objects::field::field_type::INT:
                    return consumer(static_cast<std::int32_t>(offset));
                case objects::field::field_type::FLOAT:
                    return consumer(static_cast<float>(offset));
                case objects::field::field_type::LONG:
                    return consumer(static_cast<std::int64_t>(offset));
                case objects::field::field_type::DOUBLE:
                    return consumer(static_cast<double>(offset));
                }
                return false;
            }
            template <typename after>
            inline bool read_imm_integer(after consumer, std::uint16_t offset, objects::field::field_type tp)
            {
                switch (tp)
                {
                default:
                    break;
                case objects::field::field_type::CHAR:
                    return consumer(static_cast<std::int8_t>(offset));
                case objects::field::field_type::SHORT:
                    return consumer(static_cast<std::int16_t>(offset));
                case objects::field::field_type::INT:
                    return consumer(static_cast<std::int32_t>(offset));
                case objects::field::field_type::LONG:
                    return consumer(static_cast<std::int64_t>(offset));
                }
                return false;
            }

            template<typename after>
            inline bool decode_eq(after consumer, std::uint16_t src1, objects::field::field_type t1, std::uint16_t src2, objects::field::field_type t2) {
                if (t1 == objects::field::field_type::OBJECT and t2 == objects::field::field_type::OBJECT) {
                    auto frame = this->memory_manager.stack();
                    return consumer(frame.read<objects::object>(src1), frame.read<objects::object>(src2));
                } else {
                    return this->read_stack_primitive([this, consumer, src2, t2](auto arg1){return this->read_stack_primitive([consumer, arg1](auto arg2){return consumer(arg1, arg2);}, src2, t2);}, src1, t1);
                }
            }

            template<typename after>
            inline bool decode_eq_imm(after consumer, std::uint16_t src1, objects::field::field_type t1, std::uint16_t src2, objects::field::field_type t2) {
                if (t1 == objects::field::field_type::OBJECT and t2 == objects::field::field_type::OBJECT) {
                    auto frame = this->memory_manager.stack();
                    return consumer(frame.read<objects::object>(src1), objects::object(nullptr));
                } else {
                    return this->read_stack_primitive([this, consumer, src2, t2](auto arg1){return this->read_imm_primitive([consumer, arg1](auto arg2){return consumer(arg1, arg2);}, src2, t2);}, src1, t1);
                }
            }
#pragma endregion
        private: //Writeback
#pragma region
            template <typename result_t>
            inline bool writeback(result_t result, std::uint16_t dest)
            {
                this->memory_manager.stack().write(dest, result);
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

            inline bool branch(bool jump, std::uint16_t offset, signed char forward)
            {
                return !jump || this->jump(offset, forward);
            }

#pragma endregion
        };
    } // namespace vm
} // namespace oops
#endif