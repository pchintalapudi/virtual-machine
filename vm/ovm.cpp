#include "ovm.h"
#include "../punning/puns.h"
#include "primitive_ops.h"
#include "object_ops.h"

using namespace oops::vm;

namespace
{
    enum class OPCODE
    {
        LLI = 0x40,
        CAST,
        NEG,
        DIVUI,
        DIVI,
        MULI,
        ADDI,
        DIVU,
        DIV,
        MUL,
        SUB,
        ADD,
        SRAI = 0x30,
        SRLI,
        SLLI,
        XORI,
        ORI,
        ANDI,
        SRA,
        SRL,
        SLL,
        XOR,
        OR,
        AND,
        JUMP = 0x20,
        BA,
        BNEQI,
        BEQI,
        BGTI,
        BLEI,
        BLTI,
        BGEI,
        BNEQ,
        BEQ,
        BGT,
        BLE,
        BLT,
        BGE,
        NOP = 0x00,
        RET,
        VINV,
        SINV,
        IINV,
        STLD,
        STSR,
        LSTLD,
        LSTSR,
        VLLD,
        VLSR,
        LVINV,
        LIINV,
        LVLLD,
        LVLSR,
        CVLLD,
        CVLSR,
        POPH,
        ADDH,
        TRW,
        NEW,
        IOF,
        SZE
    };
    typedef oops::objects::field::field_type type;

    template <typename after, typename source>
    bool read(after consumer, source src, std::uint32_t offset, type tp)
    {
        switch (tp)
        {
        case type::METHOD:
            return false;
        case type::OBJECT:
            return consumer(src.template read<oops::objects::object>(offset));
        case type::CHAR:
            return consumer(src.template read<std::int8_t>(offset));
        case type::SHORT:
            return consumer(src.template read<std::int16_t>(offset));
        case type::INT:
            return consumer(src.template read<std::int32_t>(offset));
        case type::LONG:
            return consumer(src.template read<std::int64_t>(offset));
        case type::FLOAT:
            return consumer(src.template read<float>(offset));
        case type::DOUBLE:
            return consumer(src.template read<double>(offset));
        }
        return false;
    }
} // namespace

int virtual_machine::execute()
{
    std::uint64_t instr = *this->next_instruction.back();
    std::uint16_t dest = instr >> 48u & 0xffffull,
                  src1 = instr >> (sizeof(std::uint16_t) << 4u) & 0xffffull,
                  src2 = instr >> (sizeof(std::uint16_t) << 3u) & 0xffffull;
    std::uint8_t typeinfo = instr >> (sizeof(uint8_t) << 3u) & 0xffull,
                 opcode = 0xffull;
    ::type t1 = static_cast<::type>(typeinfo & 0b111),
           t2 = static_cast<::type>(typeinfo >> 3 & 0b111);

    switch (static_cast<OPCODE>(opcode))
    {
#define op case OPCODE::
//Arithmetic
#pragma region
        op ADD : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_stack_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::add(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SUB : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_stack_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::sub(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op MUL : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_stack_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::mul(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op DIV : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_stack_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::div(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op DIVU : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::divu(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op ADDI : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_imm_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::add(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op MULI : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_imm_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::mul(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op DIVI : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_imm_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::div(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op DIVUI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::divu(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op NEG : if (!this->read_stack_primitive([this, dest](auto arg1) { return this->writeback(primitives::neg(arg1), dest); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op CAST : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_imm_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::cast<decltype(arg1), decltype(arg2)>(arg1), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op LLI : if (!this->read_imm_primitive_32([this, dest](auto imm) { return this->writeback(imm, dest); }, static_cast<std::uint32_t>(src1) << 16 | src2, t1)) return this->invalid_bytecode(instr);
        break;
#pragma endregion
//Bit operations
#pragma region
        op AND : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_and(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op OR : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_or(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op XOR : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_xor(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SLL : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_sll(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SRL : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_srl(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SRA : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_sra(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op ANDI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_and(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op ORI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_or(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op XORI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_xor(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SLLI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_sll(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SRLI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_srl(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SRAI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_sra(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
#pragma endregion
//Branches
#pragma region
        op BGE : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_stack_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::ge(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BLT : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_stack_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::lt(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BLE : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_stack_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::le(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BGT : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_stack_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::gt(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BGEI : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_imm_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::ge(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BLTI : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_imm_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::lt(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BLEI : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_imm_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::le(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BGTI : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_imm_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::gt(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BEQ : if (!this->decode_eq([this, dest, typeinfo](auto arg1, auto arg2) { return this->branch(arg1 == arg2, dest, typeinfo >> 6 & 1); }, src1, t1, src2, t2)) return this->invalid_bytecode(instr);
        return 0;
        op BNEQ : if (!this->decode_eq([this, dest, typeinfo](auto arg1, auto arg2) { return this->branch(arg1 != arg2, dest, typeinfo >> 6 & 1); }, src1, t1, src2, t2)) return this->invalid_bytecode(instr);
        return 0;
        op BEQI : if (!this->decode_eq_imm([this, dest, typeinfo](auto arg1, auto arg2) { return this->branch(arg1 == arg2, dest, typeinfo >> 6 & 1); }, src1, t1, src2, t2)) return this->invalid_bytecode(instr);
        return 0;
        op BNEQI : if (!this->decode_eq_imm([this, dest, typeinfo](auto arg1, auto arg2) { return this->branch(arg1 != arg2, dest, typeinfo >> 6 & 1); }, src1, t1, src2, t2)) return this->invalid_bytecode(instr);
        return 0;
        op BA : if (!this->read_stack_integer([this, typeinfo](auto offset) { return this->jump(offset, typeinfo >> 6 & 1); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op JUMP : this->jump(dest, typeinfo >> 6 & 1);
        return 0;
#pragma endregion
        //Objects
        op VLLD:
        {
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            if (!::read([frame, dest](auto val) mutable {frame.write(dest, val);return true; }, object, src2, t2))
                return this->invalid_bytecode(instr);
            break;
        }
        op VLSR:
        {
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            if (!::read([object, dest, this](auto val) mutable {vm::objects::write_object(object, dest, val, this->memory_manager);return true; }, frame, src2, t2))
                return this->invalid_bytecode(instr);
            break;
        }
        op LVLLD:
        {
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            if (!this->read_stack_integer([frame, dest, object, t2](auto offset) mutable { return ::read([frame, dest](auto val) mutable {frame.write(dest, val);return true; }, object, offset, t2); }, src2, t1))
                return this->invalid_bytecode(instr);
            break;
        }
        op LVLSR:
        {
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            if (!this->read_stack_integer([frame, src2, object, t2, this](auto offset) mutable { return ::read([object, offset, this](auto val) mutable {vm::objects::write_object(object, offset, val, this->memory_manager);return true; }, frame, src2, t2); }, dest, t1))
                return this->invalid_bytecode(instr);
            break;
        }
        op CVLLD:
        {
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            std::uint32_t offset;
            switch (t1)
            {
            default:
                return this->invalid_bytecode(instr);
            case ::type::CHAR:
            {
                offset = frame.read<std::int8_t>(src2);
                break;
            }
            case ::type::SHORT:
            {
                offset = frame.read<std::int16_t>(src2);
                break;
            }
            case ::type::INT:
            {
                offset = frame.read<std::int32_t>(src2);
                break;
            }
            }
            std::uint64_t cmp = offset;
            switch (t2)
            {
            case ::type::CHAR:
                break;
            case ::type::SHORT:
                cmp <<= 1;
                break;
            case ::type::INT:
            case ::type::FLOAT:
                cmp <<= 2;
                break;
            case ::type::LONG:
            case ::type::DOUBLE:
            case ::type::OBJECT:
                cmp <<= 3;
                break;
            default:
                return this->invalid_bytecode(instr);
            }
            if (cmp >= object.size())
            {
                //TODO throw exception
                return 1;
            }
            ::read([frame, dest](auto val) mutable {frame.write(dest, val);return true; }, object, offset, t2);
            break;
        }
        op CVLSR:
        {
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            std::uint32_t offset;
            switch (t1)
            {
            default:
                return this->invalid_bytecode(instr);
            case ::type::CHAR:
            {
                offset = frame.read<std::int8_t>(src2);
                break;
            }
            case ::type::SHORT:
            {
                offset = frame.read<std::int16_t>(src2);
                break;
            }
            case ::type::INT:
            {
                offset = frame.read<std::int32_t>(src2);
                break;
            }
            }
            std::uint64_t cmp = offset;
            switch (t2)
            {
            case ::type::CHAR:
                break;
            case ::type::SHORT:
                cmp <<= 1;
                break;
            case ::type::INT:
            case ::type::FLOAT:
                cmp <<= 2;
                break;
            case ::type::LONG:
            case ::type::DOUBLE:
            case ::type::OBJECT:
                cmp <<= 3;
                break;
            default:
                return this->invalid_bytecode(instr);
            }
            if (cmp >= object.size())
            {
                //TODO throw exception
                return 1;
            }
            ::read([object, offset, this](auto val) mutable {vm::objects::write_object(object, offset, val, this->memory_manager);return true; }, frame, src2, t2);
            break;
        }
        op STLD:
        {
            auto frame = this->memory_manager.stack();
            auto cls = frame.lookup_method().owner().get_import(src1);
            if (!::read([frame, dest](auto value) mutable {frame.write(dest, value);return true; }, cls, src2, t2))
                return this->invalid_bytecode(instr);
            break;
        }
        op STSR:
        {
            auto frame = this->memory_manager.stack();
            auto cls = frame.lookup_method().owner().get_import(src1);
            if (!::read([cls, dest](auto value) mutable {cls.write(dest, value);return true; }, frame, src2, t2))
                return this->invalid_bytecode(instr);
            break;
        }
        op LSTLD:
        {
            auto frame = this->memory_manager.stack();
            std::uint32_t offset;
            switch (t1)
            {
            default:
                return this->invalid_bytecode(instr);
            case ::type::CHAR:
                offset = frame.read<std::int8_t>(src1);
                break;
            case ::type::SHORT:
                offset = frame.read<std::int16_t>(src1);
                break;
            case ::type::INT:
                offset = frame.read<std::int32_t>(src1);
                break;
            }
            auto cls = frame.lookup_method().owner().get_import(offset);
            if (!::read([frame, dest](auto value) mutable {frame.write(dest, value);return true; }, cls, src2, t2))
                return this->invalid_bytecode(instr);
            break;
        }
        op LSTSR:
        {
            auto frame = this->memory_manager.stack();
            std::uint32_t offset;
            switch (t1)
            {
            default:
                return this->invalid_bytecode(instr);
            case ::type::CHAR:
                offset = frame.read<std::int8_t>(src1);
                break;
            case ::type::SHORT:
                offset = frame.read<std::int16_t>(src1);
                break;
            case ::type::INT:
                offset = frame.read<std::int32_t>(src1);
                break;
            }
            auto cls = frame.lookup_method().owner().get_import(offset);
            if (!::read([cls, dest](auto value) mutable {cls.write(dest, value);return true; }, frame, src2, t2))
                return this->invalid_bytecode(instr);
            break;
        }
        op SZE:
        {
            auto frame = this->memory_manager.stack();
            frame.write<std::int32_t>(src2, frame.read<oops::objects::object>(src1).size() >> 3);
            break;
        }
        op NEW:
        {
            auto frame = this->memory_manager.stack();
            auto object = this->memory_manager.new_object(frame.lookup_method().owner().get_import(static_cast<std::uint32_t>(src1) << 16 | src2));
            if (!object)
            {
                //TODO throw OOM exception
                return 1;
            }
            frame.write(dest, object.value);
            return 0;
        }
        op IOF:
        {
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            char iof = this->memory_manager.instanceof (object.get_class(), frame.lookup_method().owner().get_import(src2));
            switch (t2)
            {
            case ::type::CHAR:
                frame.write<std::int8_t>(dest, iof);
                break;
            case ::type::SHORT:
                frame.write<std::int16_t>(dest, iof);
                break;
            case ::type::INT:
                frame.write<std::int32_t>(dest, iof);
                break;
            case ::type::LONG:
                frame.write<std::int64_t>(dest, iof);
                break;
            default:
                return this->invalid_bytecode(instr);
                break;
            }
            break;
        }
        //Exceptions
        //TODO TRW
        //TODO HANDLERS?
        //Methods
        op VINV:
        {
            auto instr = this->next_instruction.back();
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            auto method = object.get_virtual_method(src2);
            auto ret = this->memory_manager.call_instance(method, object, dest, instr);
            if (!ret)
            {
                if (ret.value)
                {
                    return this->invalid_bytecode(instr);
                }
                else
                {
                    //TODO throw stack overflow error
                    return 1;
                }
            }
            this->next_instruction.back() = ret.value;
            return 0;
        }
        op LVINV:
        {
            auto instr = this->next_instruction.back();
            auto frame = this->memory_manager.stack();
            auto object = frame.read<oops::objects::object>(src1);
            std::uint32_t offset;
            switch (t2)
            {
            default:
                return this->invalid_bytecode(instr);
            case ::type::CHAR:
            {
                offset = frame.read<std::int8_t>(src2);
                break;
            }
            case ::type::SHORT:
            {
                offset = frame.read<std::int16_t>(src2);
                break;
            }
            case ::type::INT:
            {
                offset = frame.read<std::int32_t>(src2);
                break;
            }
            }
            auto method = object.get_virtual_method(offset);
            auto ret = this->memory_manager.call_instance(method, object, dest, instr);
            if (!ret)
            {
                if (ret.value)
                {
                    return this->invalid_bytecode(instr);
                }
                else
                {
                    //TODO throw stack overflow error
                    return 1;
                }
            }
            this->next_instruction.back() = ret.value;
            return 0;
        }
        op SINV:
        {
            auto instr = this->next_instruction.back();
            auto frame = this->memory_manager.stack();
            auto method = frame.lookup_method().owner().get_method(static_cast<std::uint32_t>(src1) << 16 | src2);
            auto ret = this->memory_manager.call_static(method, dest, instr);
            if (!ret)
            {
                if (ret.value)
                {
                    return this->invalid_bytecode(instr);
                }
                else
                {
                    //TODO throw stack overflow error
                    return -1;
                }
            }
            this->next_instruction.back() = ret.value;
            return 0;
        }
        op IINV:
        {
            auto invoke_instr = this->next_instruction.back();
            auto frame = this->memory_manager.stack();
            auto imethod = frame.lookup_method().owner().get_method(src1);
            auto object = frame.read<oops::objects::object>(src2);
            auto maybe_method = this->memory_manager.lookup_interface(imethod, object.get_class());
            if (maybe_method)
            {
                auto ret = this->memory_manager.call_instance(maybe_method.value, object, dest, invoke_instr);
                if (!ret)
                {
                    if (ret.value)
                    {
                        return this->invalid_bytecode(instr);
                    }
                    else
                    {
                        //TODO throw stack overflow error
                        return 1;
                    }
                }
                this->next_instruction.back() = ret.value;
                return 0;
            }
            else
            {
                //TODO throw method lookup failed exception
                return 1;
            }
        }
        op LIINV:
        {
            auto invoke_instr = this->next_instruction.back();
            auto frame = this->memory_manager.stack();
            std::uint32_t offset;
            switch (t1)
            {
            case ::type::CHAR:
                offset = frame.read<std::int8_t>(src1);
                break;
            case ::type::SHORT:
                offset = frame.read<std::int16_t>(src1);
                break;
            case ::type::INT:
                offset = frame.read<std::int32_t>(src1);
                break;
            default:
                return this->invalid_bytecode(instr);
            }
            auto imethod = frame.lookup_method().owner().get_method(offset);
            auto object = frame.read<oops::objects::object>(src2);
            auto maybe_method = this->memory_manager.lookup_interface(imethod, object.get_class());
            if (maybe_method)
            {
                auto ret = this->memory_manager.call_instance(maybe_method.value, object, dest, invoke_instr);
                if (!ret)
                {
                    if (ret.value)
                    {
                        return this->invalid_bytecode(instr);
                    }
                    else
                    {
                        //TODO throw stack overflow error
                        return 1;
                    }
                }
                this->next_instruction.back() = ret.value;
                return 0;
            }
            else
            {
                //TODO throw method lookup failed exception
                return 1;
            }
        }
        op RET:
        {
            this->memory_manager.ret(src1);
            break;
        }
        op NOP : break;
    default:
        return this->invalid_bytecode(instr);
    }
    this->next_instruction.back()++;
    return 0;
}