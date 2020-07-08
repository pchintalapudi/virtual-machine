#include "vm.h"

#include <algorithm>

#include "primitives.h"
#include "../bytecode/instruction.h"

using namespace oops::virtual_machine;

oops::objects::clazz virtual_machine::current_class() {
    return this->frame.get_method().enclosing_class();
}

int virtual_machine::exec_loop()
{
    using itype = bytecode::instruction::type;
    std::uint64_t depth = 0;
    while (true)
    {
        bytecode::instruction instruction(this->ip);
        switch (instruction.get_type())
        {
        case itype::NOP:
            break;
#pragma region //Basic ops

#define basic_op(opcode, op, type)                                                                                                                     \
    case itype::opcode:                                                                                                                                \
        this->frame.write(instruction.dest(), primitives::op(this->frame.read<type>(instruction.src1()), this->frame.read<type>(instruction.src2()))); \
        break;
#define basic_op_imm(opcode, op, type)                                                                                                            \
    case itype::opcode:                                                                                                                           \
        this->frame.write(instruction.dest(), primitives::op(this->frame.read<type>(instruction.src1()), static_cast<type>(instruction.imm2()))); \
        break;
            basic_op(IADD, add, std::int32_t);
            basic_op(LADD, add, std::int64_t);
            basic_op(FADD, add, float);
            basic_op(DADD, add, double);
            basic_op(ISUB, sub, std::int32_t);
            basic_op(LSUB, sub, std::int64_t);
            basic_op(FSUB, sub, float);
            basic_op(DSUB, sub, double);
            basic_op(IMUL, mul, std::int32_t);
            basic_op(LMUL, mul, std::int64_t);
            basic_op(FMUL, mul, float);
            basic_op(DMUL, mul, double);
            basic_op(IDIV, div, std::int32_t);
            basic_op(LDIV, div, std::int64_t);
            basic_op(FDIV, div, float);
            basic_op(DDIV, div, double);
            basic_op(IDIVU, divu, std::int32_t);
            basic_op(LDIVU, divu, std::int64_t);
            basic_op_imm(INEG, neg, std::int32_t);
            basic_op_imm(LNEG, neg, std::int64_t);
            basic_op_imm(FNEG, neg, float);
            basic_op_imm(DNEG, neg, double);
            basic_op_imm(IADDI, add, std::int32_t);
            basic_op_imm(LADDI, add, std::int64_t);
            basic_op_imm(FADDI, add, float);
            basic_op_imm(DADDI, add, double);
            basic_op_imm(ISUBI, sub, std::int32_t);
            basic_op_imm(LSUBI, sub, std::int64_t);
            basic_op_imm(FSUBI, sub, float);
            basic_op_imm(DSUBI, sub, double);
            basic_op_imm(IMULI, mul, std::int32_t);
            basic_op_imm(LMULI, mul, std::int64_t);
            basic_op_imm(FMULI, mul, float);
            basic_op_imm(DMULI, mul, double);
            basic_op_imm(IDIVI, div, std::int32_t);
            basic_op_imm(LDIVI, div, std::int64_t);
            basic_op_imm(FDIVI, div, float);
            basic_op_imm(DDIVI, div, double);
            basic_op_imm(IDIVUI, divu, std::int32_t);
            basic_op_imm(LDIVUI, divu, std::int64_t);
            basic_op(IAND, band, std::int32_t);
            basic_op(LAND, band, std::int64_t);
            basic_op(IOR, bor, std::int32_t);
            basic_op(LOR, bor, std::int64_t);
            basic_op(IXOR, bxor, std::int32_t);
            basic_op(LXOR, bxor, std::int64_t);
            basic_op(ISLL, bsll, std::int32_t);
            basic_op(LSLL, bsll, std::int64_t);
            basic_op(ISRL, bsrl, std::int32_t);
            basic_op(LSRL, bsrl, std::int64_t);
            basic_op(ISRA, bsra, std::int32_t);
            basic_op(LSRA, bsra, std::int64_t);
            basic_op_imm(IANDI, band, std::int32_t);
            basic_op_imm(LANDI, band, std::int64_t);
            basic_op_imm(IORI, bor, std::int32_t);
            basic_op_imm(LORI, bor, std::int64_t);
            basic_op_imm(IXORI, bxor, std::int32_t);
            basic_op_imm(LXORI, bxor, std::int64_t);
            basic_op_imm(ISLLI, bsll, std::int32_t);
            basic_op_imm(LSLLI, bsll, std::int64_t);
            basic_op_imm(ISRLI, bsrl, std::int32_t);
            basic_op_imm(LSRLI, bsrl, std::int64_t);
            basic_op_imm(ISRAI, bsra, std::int32_t);
            basic_op_imm(LSRAI, bsra, std::int64_t);
#undef basic_op
#undef basic_op_imm
#pragma endregion
#pragma region //Casts

#define cast_op(opcode, from, to)                                                                                      \
    case itype::opcode:                                                                                                \
        this->frame.write(instruction.dest(), primitives::cast<from, to>(this->frame.read<from>(instruction.src1()))); \
        break;
            cast_op(ICSTL, std::int32_t, std::int64_t);
            cast_op(ICSTF, std::int32_t, float);
            cast_op(ICSTD, std::int32_t, double);
            cast_op(LCSTI, std::int64_t, std::int32_t);
            cast_op(LCSTF, std::int64_t, float);
            cast_op(LCSTD, std::int64_t, double);
            cast_op(FCSTI, float, std::int32_t);
            cast_op(FCSTL, float, std::int64_t);
            cast_op(FCSTD, float, double);
            cast_op(DCSTI, double, std::int32_t);
            cast_op(DCSTL, double, std::int64_t);
            cast_op(DCSTF, double, float);
#undef cast_op
#pragma endregion
#pragma region //Long immediates

        case itype::LLI:
        {
            this->frame.write(instruction.dest(), instruction.imm32());
            break;
        }
        case itype::LUI:
        {
            this->frame.write(instruction.dest(), instruction.imm32());
            break;
        }
        case itype::LNL:
        {
            this->frame.write(instruction.dest(), objects::object(nullptr));
            break;
        }
#pragma endregion
#pragma region //Branches

#define branch_op(opcode, preop, op, type)                                                                                                                                                                                                                                                                                                                        \
    case itype::opcode:                                                                                                                                                                                                                                                                                                                                           \
        this->ip = preop primitives::op(this->frame.read<type>(instruction.src1()), this->frame.read<type>(instruction.src2())) ? instruction.flags() ? this->ip + static_cast<std::uint32_t>(instruction.dest()) * sizeof(std::uint64_t) : this->ip - static_cast<std::uint32_t>(instruction.dest()) * sizeof(std::uint64_t) : this->ip + sizeof(std::uint64_t); \
        continue;
#define branch_op_imm(opcode, preop, op, type)                                                                                                                                                                                                                                                                                                               \
    case itype::opcode:                                                                                                                                                                                                                                                                                                                                      \
        this->ip = preop primitives::op(this->frame.read<type>(instruction.src1()), static_cast<type>(instruction.imm2())) ? instruction.flags() ? this->ip + static_cast<std::uint32_t>(instruction.dest()) * sizeof(std::uint64_t) : this->ip - static_cast<std::uint32_t>(instruction.dest()) * sizeof(std::uint64_t) : this->ip + sizeof(std::uint64_t); \
        continue;
#define vbranch_op_imm(opcode, preop)                                                                                                                                                                                                                                                                                                                                \
    case itype::opcode:                                                                                                                                                                                                                                                                                                                                              \
        this->ip = preop primitives::eq(this->frame.read<objects::base_object>(instruction.src1()), objects::base_object(nullptr)) ? instruction.flags() ? this->ip + static_cast<std::uint32_t>(instruction.dest()) * sizeof(std::uint64_t) : this->ip - static_cast<std::uint32_t>(instruction.dest()) * sizeof(std::uint64_t) : this->ip + sizeof(std::uint64_t); \
        continue;
            branch_op(IBGE, !, lt, std::int32_t);
            branch_op(IBLT, , lt, std::int32_t);
            branch_op(IBLE, !, gt, std::int32_t);
            branch_op(IBGT, , gt, std::int32_t);
            branch_op(IBEQ, , eq, std::int32_t);
            branch_op(IBNEQ, !, eq, std::int32_t);
            branch_op(LBGE, !, lt, std::int64_t);
            branch_op(LBLT, , lt, std::int64_t);
            branch_op(LBLE, !, gt, std::int64_t);
            branch_op(LBGT, , gt, std::int64_t);
            branch_op(LBEQ, , eq, std::int64_t);
            branch_op(LBNEQ, !, eq, std::int64_t);
            branch_op(FBGE, !, lt, float);
            branch_op(FBLT, , lt, float);
            branch_op(FBLE, !, gt, float);
            branch_op(FBGT, , gt, float);
            branch_op(FBEQ, , eq, float);
            branch_op(FBNEQ, !, eq, float);
            branch_op(DBGE, !, lt, double);
            branch_op(DBLT, , lt, double);
            branch_op(DBLE, !, gt, double);
            branch_op(DBGT, , gt, double);
            branch_op(DBEQ, , eq, double);
            branch_op(DBNEQ, !, eq, double);
            branch_op_imm(IBGEI, !, lt, std::int32_t);
            branch_op_imm(IBLTI, , lt, std::int32_t);
            branch_op_imm(IBLEI, !, gt, std::int32_t);
            branch_op_imm(IBGTI, , gt, std::int32_t);
            branch_op_imm(IBEQI, , eq, std::int32_t);
            branch_op_imm(IBNEQI, !, eq, std::int32_t);
            branch_op_imm(LBGEI, !, lt, std::int64_t);
            branch_op_imm(LBLTI, , lt, std::int64_t);
            branch_op_imm(LBLEI, !, gt, std::int64_t);
            branch_op_imm(LBGTI, , gt, std::int64_t);
            branch_op_imm(LBEQI, , eq, std::int64_t);
            branch_op_imm(LBNEQI, !, eq, std::int64_t);
            branch_op_imm(FBGEI, !, lt, float);
            branch_op_imm(FBLTI, , lt, float);
            branch_op_imm(FBLEI, !, gt, float);
            branch_op_imm(FBGTI, , gt, float);
            branch_op_imm(FBEQI, , eq, float);
            branch_op_imm(FBNEQI, !, eq, float);
            branch_op_imm(DBGEI, !, lt, double);
            branch_op_imm(DBLTI, , lt, double);
            branch_op_imm(DBLEI, !, gt, double);
            branch_op_imm(DBGTI, , gt, double);
            branch_op_imm(DBEQI, , eq, double);
            branch_op_imm(DBNEQI, !, eq, double);
            branch_op(VBEQ, , eq, objects::base_object);
            branch_op(VBNEQ, !, eq, objects::base_object);
            vbranch_op_imm(VBEQI, );
            vbranch_op_imm(VBNEQI, !);
#undef branch_op
#undef branch_op_imm
#undef vbranch_op_imm
        case itype::BU:
            this->ip = instruction.flags() ? this->ip + instruction.dest() : this->ip - instruction.dest();
            continue;
        case itype::BADR:
        {
            std::uint32_t offset = this->frame.read<std::int32_t>(instruction.dest());
            this->ip = offset <= static_cast<std::uint32_t>(instruction.src2()) and offset >= static_cast<std::uint32_t>(instruction.src1()) ? this->ip + utils::pun_read<std::uint16_t>(this->ip + sizeof(std::uint64_t) + offset * sizeof(std::uint16_t)) : this->ip + utils::pun_read<std::uint16_t>(this->ip + sizeof(std::uint64_t) + (static_cast<std::uint32_t>(instruction.src2()) + 1) * sizeof(std::uint16_t));
            continue;
        }
#define bcmp(opcode, type)                                                                                                                                         \
    case itype::opcode:                                                                                                                                            \
    {                                                                                                                                                              \
        auto cmp_length = instruction.src1();                                                                                                                      \
        auto cmp_value = this->frame.read<type>(instruction.src2());                                                                                               \
        utils::const_aliased_iterator<type> begin(this->ip + sizeof(std::uint64_t)), end(begin + cmp_length);                                                      \
        auto found = std::lower_bound(begin, end, cmp_value);                                                                                                      \
        if (found != end && *found == cmp_value)                                                                                                                   \
        {                                                                                                                                                          \
            auto ip_diff = utils::pun_read<std::uint16_t>(this->ip + sizeof(std::uint64_t) + cmp_length * sizeof(type) + (found - begin) * sizeof(std::uint16_t)); \
            this->ip = instruction.flags() ? this->ip + ip_diff : this->ip - ip_diff;                                                                              \
        }                                                                                                                                                          \
        else                                                                                                                                                       \
        {                                                                                                                                                          \
            auto ip_diff = utils::pun_read<std::uint16_t>(this->ip + sizeof(std::uint64_t) + cmp_length * sizeof(type) + cmp_length * sizeof(std::uint16_t));      \
            this->ip = instruction.flags() ? this->ip + ip_diff : this->ip - ip_diff;                                                                              \
        }                                                                                                                                                          \
        continue;                                                                                                                                                  \
    }
            bcmp(IBCMP, std::int32_t);
            bcmp(LBCMP, std::int64_t);
            bcmp(FBCMP, float);
            bcmp(DBCMP, double);
#undef bcmp
#pragma endregion
#pragma region //Memory allocation

        case itype::VNEW:
        {
            auto maybe_object = this->heap.allocate_object(this->current_class().lookup_class(instruction.imm24()));
            if (maybe_object)
            {
                this->frame.write(instruction.dest(), *maybe_object);
            }
            else
            {
                //TODO throw OOM
            }
            break;
        }
//TODO throw OOM on allocation failure
#define array_new(opcode, field_type)                                                                                                       \
    case itype::opcode:                                                                                                                     \
    {                                                                                                                                       \
        auto maybe_array = this->heap.allocate_array(objects::field::type::field_type, this->frame.read<std::int32_t>(instruction.src1())); \
        if (maybe_array)                                                                                                                    \
        {                                                                                                                                   \
            this->frame.write(instruction.dest(), *maybe_array);                                                                            \
        }                                                                                                                                   \
        break;                                                                                                                              \
    }
            array_new(CANEW, CHAR);
            array_new(SANEW, SHORT);
            array_new(IANEW, INT);
            array_new(LANEW, LONG);
            array_new(FANEW, FLOAT);
            array_new(DANEW, DOUBLE);
            array_new(VANEW, OBJECT);
#undef array_new
        case itype::IOF:
            this->frame.write<std::int32_t>(instruction.dest(), this->instanceof (this->current_class().lookup_class(instruction.src1()), this->frame.read<objects::base_object>(instruction.src2()).get_clazz()));
            break;
        case itype::LIOF:
            this->frame.write<std::int32_t>(instruction.dest(), this->instanceof (this->current_class().lookup_class(this->frame.read<std::int32_t>(instruction.src1())), this->frame.read<objects::base_object>(instruction.src2()).get_clazz()));
            break;
#pragma endregion
#pragma region //Load/store

#define vlld(opcode, ctype, type)                                                                                                           \
    case itype::opcode:                                                                                                                     \
        this->frame.write<ctype>(instruction.dest(), this->frame.read<objects::object>(instruction.src1()).read<type>(instruction.src2())); \
        break;
            vlld(CVLLD, std::int32_t, std::int8_t);
            vlld(SVLLD, std::int32_t, std::int16_t);
            vlld(IVLLD, std::int32_t, std::int32_t);
            vlld(LVLLD, std::int64_t, std::int64_t);
            vlld(FVLLD, float, float);
            vlld(DVLLD, double, double);
            vlld(VVLLD, objects::base_object, objects::base_object);
#undef vlld
#define vlsr(opcode, ctype, type)                                                                                                           \
    case itype::opcode:                                                                                                                     \
        this->frame.read<objects::object>(instruction.src1()).write<type>(instruction.dest(), this->frame.read<ctype>(instruction.src2())); \
        break;
            vlsr(CVLSR, std::int32_t, std::int8_t);
            vlsr(SVLSR, std::int32_t, std::int16_t);
            vlsr(IVLSR, std::int32_t, std::int32_t);
            vlsr(LVLSR, std::int64_t, std::int64_t);
            vlsr(FVLSR, float, float);
            vlsr(DVLSR, double, double);
            vlsr(VVLSR, objects::base_object, objects::base_object);
#undef vlsr
//TODO throw array out of bounds exception
#define alld(opcode, ctype, type)                                                                                             \
    case itype::opcode:                                                                                                       \
    {                                                                                                                         \
        auto array = this->frame.read<objects::array>(instruction.src1());                                                    \
        std::uint32_t index = this->frame.read<std::int32_t>(instruction.src2());                                             \
        if (index < array.length())                                                                                           \
            this->frame.write<ctype>(instruction.dest(), array.read<type>(static_cast<std::uint64_t>(index) * sizeof(type))); \
        else                                                                                                                  \
        {                                                                                                                     \
        }                                                                                                                     \
        break;                                                                                                                \
    }
            alld(CALD, std::int32_t, std::int8_t);
            alld(SALD, std::int32_t, std::int16_t);
            alld(IALD, std::int32_t, std::int32_t);
            alld(LALD, std::int64_t, std::int64_t);
            alld(FALD, float, float);
            alld(DALD, double, double);
            alld(VALD, objects::base_object, objects::base_object);
#undef alld
//TODO do bounds checking
#define alsr(opcode, ctype, type)                                                                                             \
    case itype::opcode:                                                                                                       \
    {                                                                                                                         \
        auto array = this->frame.read<objects::array>(instruction.dest());                                                    \
        std::uint32_t index = this->frame.read<std::int32_t>(instruction.src2());                                             \
        if (index < array.length())                                                                                           \
            array.write<type>(static_cast<std::uint64_t>(index) * sizeof(type), this->frame.read<ctype>(instruction.src1())); \
        else                                                                                                                  \
        {                                                                                                                     \
        }                                                                                                                     \
        break;                                                                                                                \
    }
            alsr(CASR, std::int32_t, std::int8_t);
            alsr(SASR, std::int32_t, std::int16_t);
            alsr(IASR, std::int32_t, std::int32_t);
            alsr(LASR, std::int64_t, std::int64_t);
            alsr(FASR, float, float);
            alsr(DASR, double, double);
            alsr(VASR, objects::base_object, objects::base_object);
#undef alsr
#define stld(opcode, ctype, type)                                                                                                             \
    case itype::opcode:                                                                                                                       \
        this->frame.write<ctype>(instruction.dest(), this->current_class().lookup_class(instruction.imm24()).read<type>(instruction.src1())); \
        break;
            stld(CSTLD, std::int32_t, std::int8_t);
            stld(SSTLD, std::int32_t, std::int16_t);
            stld(ISTLD, std::int32_t, std::int32_t);
            stld(LSTLD, std::int64_t, std::int64_t);
            stld(FSTLD, float, float);
            stld(DSTLD, double, double);
            stld(VSTLD, objects::base_object, objects::base_object);
#undef stld
#define stsr(opcode, ctype, type)                                                                                                             \
    case itype::opcode:                                                                                                                       \
        this->current_class().lookup_class(instruction.imm24()).write<type>(instruction.dest(), this->frame.read<ctype>(instruction.src1())); \
        break;
            stsr(CSTSR, std::int32_t, std::int8_t);
            stsr(SSTSR, std::int32_t, std::int16_t);
            stsr(ISTSR, std::int32_t, std::int32_t);
            stsr(LSTSR, std::int64_t, std::int64_t);
            stsr(FSTSR, float, float);
            stsr(DSTSR, double, double);
            stsr(VSTSR, objects::base_object, objects::base_object);
#undef stsr

#pragma endregion
#pragma region //Methods

#define ret(opcode, type)                                                                              \
    case itype::opcode:                                                                                \
    {                                                                                                  \
        std::uint16_t return_offset = this->frame.return_offset();                                     \
        std::uint32_t return_address = this->frame.return_address();                                   \
        auto value = this->frame.read<type>(instruction.dest());                                       \
        this->stack.load_and_pop(this->frame);                                                         \
        this->ip = this->frame.get_method().bytecode_begin() + return_address * sizeof(std::uint64_t); \
        this->frame.write(return_offset, value);                                                       \
        if (!depth--)                                                                                  \
        {                                                                                              \
            return 0;                                                                                  \
        }                                                                                              \
        break;                                                                                         \
    }
            ret(IRET, std::int32_t);
            ret(LRET, std::int64_t);
            ret(FRET, float);
            ret(DRET, double);
#undef ret
        case itype::SINV:
        {
            auto method = this->current_class().lookup_method(instruction.imm32());
            if (this->stack.init_frame(this->frame, method, instruction.dest()))
            {
                this->ip = method.bytecode_begin();
                ++depth;
            }
            else
            {
                //TODO throw stack overflow error
            }
            continue;
        }
        case itype::VINV:
        {
            auto method = this->frame.read<objects::base_object>(instruction.src1()).get_clazz().lookup_method(instruction.imm24());
            if (this->stack.init_frame(this->frame, method, instruction.dest()))
            {
                this->ip = method.bytecode_begin();
                ++depth;
            }
            else
            {
                //TODO throw stack overflow error
            }
            continue;
        }
        case itype::IINV:
        {
            auto imethod = this->current_class().lookup_method(instruction.imm24());
            auto maybe_method = this->lookup_interface_method(imethod, this->frame.read<objects::base_object>(instruction.src1()));
            if (maybe_method)
            {
                auto method = *maybe_method;
                if (this->stack.init_frame(this->frame, method, instruction.dest()))
                {
                    this->ip = method.bytecode_begin();
                    ++depth;
                }
                else
                {
                    //TODO throw stack overflow error
                }
            }
            else
            {
                //TODO throw interface lookup error
            }
            continue;
        }
#pragma endregion
#pragma region //Exceptions

        case itype::EXC:
        {
            //TODO handle exceptions
        }
#pragma endregion
        }
        this->ip += sizeof(std::uint64_t);
    }
    return -1;
}