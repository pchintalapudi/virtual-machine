#include "vm.h"

#include <algorithm>

#include "primitives.h"
#include "../bytecode/instruction.h"
#include "../platform_specific/dlls.h"

using namespace oops::virtual_machine;

oops::objects::clazz virtual_machine::current_class()
{
    return this->frame.get_method().enclosing_class();
}

std::optional<oops::objects::object> virtual_machine::new_object(objects::clazz cls)
{
    auto obj = this->heap.allocate_object(cls);
    if (!obj)
    {
        this->gc(true);
        obj = this->heap.allocate_object(cls);
    }
    return obj;
}

std::optional<oops::objects::array> virtual_machine::new_array(objects::field::type atype, std::uint32_t length)
{
    auto cls = this->array_classes[static_cast<std::uint8_t>(atype)];
    std::uint64_t memory_region = objects::array::size(atype, length);
    auto obj = this->heap.allocate_array(cls, memory_region);
    if (!obj)
    {
        this->gc();
        obj = this->heap.allocate_array(cls, memory_region);
    }
    return obj;
}

std::optional<oops::objects::method> virtual_machine::lookup_interface_method(objects::method imethod, objects::base_object src)
{
    auto maybe_offset = this->class_manager.lookup_interface_method(imethod, src);
    if (maybe_offset)
    {
        return this->lookup_method_offset(src.get_clazz(), *maybe_offset);
    }
    return {};
}

bool virtual_machine:: instanceof (objects::clazz src, objects::clazz test)
{
    return this->class_manager.instanceof (src, test);
}

result virtual_machine::exec_loop()
{
    using itype = bytecode::instruction::type;
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
            auto maybe_object = this->new_object(this->lookup_class_offset(this->current_class(), instruction.imm24()));
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
#define array_new(opcode, field_type)                                                                                             \
    case itype::opcode:                                                                                                           \
    {                                                                                                                             \
        auto maybe_array = this->new_array(objects::field::type::field_type, this->frame.read<std::int32_t>(instruction.src1())); \
        if (maybe_array)                                                                                                          \
        {                                                                                                                         \
            this->frame.write(instruction.dest(), *maybe_array);                                                                  \
        }                                                                                                                         \
        break;                                                                                                                    \
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
            this->frame.write<std::int32_t>(instruction.dest(), this->instanceof (this->lookup_class_offset(this->current_class(), instruction.imm24()), this->frame.read<objects::base_object>(instruction.src1()).get_clazz()));
            break;
#pragma endregion
#pragma region //Load/store

//TODO throw array out of bounds exception
#define ald(opcode, ctype, type)                                                                                              \
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
            ald(CALD, std::int32_t, std::int8_t);
            ald(SALD, std::int32_t, std::int16_t);
            ald(IALD, std::int32_t, std::int32_t);
            ald(LALD, std::int64_t, std::int64_t);
            ald(FALD, float, float);
            ald(DALD, double, double);
            ald(VALD, objects::base_object, objects::base_object);
#undef ald
//TODO throw array out of bounds exception
#define asr(opcode, ctype, type)                                                        \
    case itype::opcode:                                                                 \
    {                                                                                   \
        auto array = this->frame.read<objects::array>(instruction.dest());              \
        std::uint32_t index = this->frame.read<std::int32_t>(instruction.src2());       \
        if (index < array.length())                                                     \
        {                                                                               \
            auto value = this->frame.read<ctype>(instruction.src1());                   \
            this->write_barrier(array, value);                                          \
            array.write<type>(static_cast<std::uint64_t>(index) * sizeof(type), value); \
        }                                                                               \
        else                                                                            \
        {                                                                               \
        }                                                                               \
        break;                                                                          \
    }
            asr(CASR, std::int32_t, std::int8_t);
            asr(SASR, std::int32_t, std::int16_t);
            asr(IASR, std::int32_t, std::int32_t);
            asr(LASR, std::int64_t, std::int64_t);
            asr(FASR, float, float);
            asr(DASR, double, double);
            asr(VASR, objects::base_object, objects::base_object);
#undef asr
//TODO throw exception
#define vlld(opcode, type)                                                                          \
    case itype::opcode:                                                                             \
    {                                                                                               \
        if (!this->virtual_load<type>(instruction.src1(), instruction.dest(), instruction.imm24())) \
        {                                                                                           \
        }                                                                                           \
        break;                                                                                      \
    }
            vlld(CVLLD, std::int8_t);
            vlld(SVLLD, std::int16_t);
            vlld(IVLLD, std::int32_t);
            vlld(LVLLD, std::int64_t);
            vlld(FVLLD, float);
            vlld(DVLLD, double);
            vlld(VVLLD, objects::base_object);
#undef vlld
//TODO throw exception
#define vlsr(opcode, type)                                                                           \
    case itype::opcode:                                                                              \
    {                                                                                                \
        if (!this->virtual_store<type>(instruction.src1(), instruction.dest(), instruction.imm24())) \
        {                                                                                            \
        }                                                                                            \
        break;                                                                                       \
    }
            vlsr(CVLSR, std::int8_t);
            vlsr(SVLSR, std::int16_t);
            vlsr(IVLSR, std::int32_t);
            vlsr(LVLSR, std::int64_t);
            vlsr(FVLSR, float);
            vlsr(DVLSR, double);
            vlsr(VVLSR, objects::base_object);
#undef vlsr
//TODO throw exception
#define stld(opcode, type)                                                     \
    case itype::opcode:                                                        \
    {                                                                          \
        if (!this->static_load<type>(instruction.dest(), instruction.imm32())) \
        {                                                                      \
        }                                                                      \
        break;                                                                 \
    }
            stld(CSTLD, std::int8_t);
            stld(SSTLD, std::int16_t);
            stld(ISTLD, std::int32_t);
            stld(LSTLD, std::int64_t);
            stld(FSTLD, float);
            stld(DSTLD, double);
            stld(VSTLD, objects::base_object);
#undef stld
#define stsr(opcode, type)                                                      \
    case itype::opcode:                                                         \
    {                                                                           \
        if (!this->static_store<type>(instruction.dest(), instruction.imm32())) \
        {                                                                       \
        }                                                                       \
        break;                                                                  \
    }
            stsr(CSTSR, std::int8_t);
            stsr(SSTSR, std::int16_t);
            stsr(ISTSR, std::int32_t);
            stsr(LSTSR, std::int64_t);
            stsr(FSTSR, float);
            stsr(DSTSR, double);
            stsr(VSTSR, objects::base_object);
#undef stsr
#pragma endregion
#pragma region //Methods

#define ret(opcode, type)                                             \
    case itype::opcode:                                               \
    {                                                                 \
        auto res = this->frame.read<type>(instruction.src1());        \
        if (!(this->ip = this->stack.load_and_pop(this->frame, res))) \
            return result(res, 0);                                    \
        break;                                                        \
    }
            ret(IRET, std::int32_t);
            ret(LRET, std::int64_t);
            ret(FRET, float);
            ret(DRET, double);
            ret(VRET, objects::base_object);
#undef ret
        case itype::SINV:
        {
            auto method = *this->lookup_method_offset(this->current_class(), instruction.imm32());
            if (this->stack.init_frame(this->frame, method, instruction.dest(), this->ip))
            {
                if (method.get_type() == objects::method::type::REGULAR)
                {
                    this->ip = method.bytecode_begin();
                }
                else
                {
                    result res = platform::invoke_native(this, method.enclosing_class(), method);
                    switch (res.get_type())
                    {
                    case objects::field::type::INT:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<std::int32_t>());
                        break;
                    case objects::field::type::LONG:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<std::int64_t>());
                        break;
                    case objects::field::type::FLOAT:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<float>());
                        break;
                    case objects::field::type::DOUBLE:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<double>());
                        break;
                    case objects::field::type::OBJECT:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<objects::base_object>());
                        break;
                    default:
                        return result(objects::base_object(nullptr), -1);
                    }
                }
            }
            else
            {
                //TODO throw stack overflow error
            }
            continue;
        }
        case itype::VINV:
        {
            auto method = *this->lookup_method_offset(this->frame.read<objects::base_object>(instruction.src1()).get_clazz(), instruction.imm24());
            switch (method.get_type())
            {
            default:
                return result(objects::base_object(nullptr), -1);
            case objects::method::type::REGULAR:
            case objects::method::type::NATIVE:
                break;
            }
            if (this->stack.init_frame(this->frame, method, instruction.dest(), this->ip))
            {
                if (method.get_type() == objects::method::type::REGULAR)
                {
                    this->ip = method.bytecode_begin();
                }
                else
                {
                    result res = platform::invoke_native(this, method.enclosing_class(), method);
                    switch (res.get_type())
                    {
                    case objects::field::type::INT:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<std::int32_t>());
                        break;
                    case objects::field::type::LONG:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<std::int64_t>());
                        break;
                    case objects::field::type::FLOAT:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<float>());
                        break;
                    case objects::field::type::DOUBLE:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<double>());
                        break;
                    case objects::field::type::OBJECT:
                        this->ip = this->stack.load_and_pop(this->frame, res.get_value<objects::base_object>());
                        break;
                    default:
                        return result(objects::base_object(nullptr), -1);
                    }
                }
            }
            else
            {
                //TODO throw stack overflow error
            }
            continue;
        }
        case itype::IINV:
        {
            auto imethod = *this->lookup_method_offset(this->current_class(), instruction.imm24());
            auto maybe_method = this->lookup_interface_method(imethod, this->frame.read<objects::base_object>(instruction.src1()));
            if (maybe_method)
            {
                auto method = *maybe_method;
                if (this->stack.init_frame(this->frame, method, instruction.dest(), this->ip))
                {
                    if (method.get_type() == objects::method::type::REGULAR)
                    {
                        this->ip = method.bytecode_begin();
                    }
                    else
                    {
                        result res = platform::invoke_native(this, method.enclosing_class(), method);
                        switch (res.get_type())
                        {
                        case objects::field::type::INT:
                            this->ip = this->stack.load_and_pop(this->frame, res.get_value<std::int32_t>());
                            break;
                        case objects::field::type::LONG:
                            this->ip = this->stack.load_and_pop(this->frame, res.get_value<std::int64_t>());
                            break;
                        case objects::field::type::FLOAT:
                            this->ip = this->stack.load_and_pop(this->frame, res.get_value<float>());
                            break;
                        case objects::field::type::DOUBLE:
                            this->ip = this->stack.load_and_pop(this->frame, res.get_value<double>());
                            break;
                        case objects::field::type::OBJECT:
                            this->ip = this->stack.load_and_pop(this->frame, res.get_value<objects::base_object>());
                            break;
                        default:
                            return result(objects::base_object(nullptr), -1);
                        }
                    }
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
}

int virtual_machine::vm_core_startup(const std::vector<utils::ostring> &args)
{
    if (args.empty())
        return -1;
    auto cls = this->class_manager.load_class(args[0]);
    if (!cls)
    {
        return -1;
    }
    char main[] = "static void main(String[])";
    char main_ostring[sizeof(main) + sizeof(std::uint32_t) - 1];
    utils::pun_write(main_ostring, static_cast<std::uint32_t>(sizeof(main) - 1));
    std::memcpy(main_ostring + sizeof(std::uint32_t), main, sizeof(main) - 1);
    std::optional<std::uint32_t> offset = cls.get_real_method_offset(utils::ostring(main_ostring + sizeof(std::uint32_t)));
    if (offset)
    {
        auto method = cls.direct_method_lookup(*offset);
        //TODO load args
        this->ip = method.bytecode_begin();
        return this->exec_loop().get_status();
    }
    else
    {
        return -1;
    }
}

result virtual_machine::execute(utils::ostring class_name, utils::ostring method_name, const std::vector<result> &args)
{
    auto cls = this->class_manager.load_class(class_name);
    if (!cls)
    {
        return result(objects::base_object(nullptr), -1);
    }
    auto offset = cls.get_real_method_offset(method_name);
    if (!offset)
    {
        return result(objects::base_object(nullptr), -1);
    }
    auto method = cls.direct_method_lookup(*offset);
    return this->execute(method, args);
}

result virtual_machine::execute(objects::method method, const std::vector<result> &args)
{
    this->ip = method.bytecode_begin();
    if (!this->stack.init_from_native_frame(this->frame, method, args))
    {
        return result(objects::base_object(nullptr), -1);
    }
    return this->exec_loop();
}