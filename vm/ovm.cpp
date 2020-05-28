#include "ovm.h"
#include "../punning/puns.h"

using namespace oops::vm;

namespace
{
    enum class OPCODE
    {
        LUI = 0x60,
        CAST,
        NEG,
        DIVI,
        MULI,
        ADDI,
        DIV,
        MUL,
        SUB,
        ADD,
        SRAI = 0x50,
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
        JUMP = 0x40,
        LA,
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
        SCONST = 0x30,
        LCONST,
        LCLASS = 0x20,
        INSTANCEOF,
        CLASSOF,
        NEW,
        STORE,
        LOAD,
        POPHDR = 0x10,
        PUSHHDR,
        THROW,
        NOP = 0x00,
        VD,
        RET,
        ARGS,
        INVOKE
    };
    typedef oops::objects::field::field_type type;
    inline bool assert_convertible_types(type t1, type t2)
    {
        switch (t1)
        {
        case type::DOUBLE:
        case type::FLOAT:
            return t2 == type::DOUBLE or t2 == type::FLOAT;
        case type::OBJECT:
            return t2 == type::OBJECT;
        case type::METHOD:
            return t2 == type::METHOD;
        default:
            return t2 == type::CHAR or t2 == type::SHORT or t2 == type::INT or t2 == type::LONG;
        }
    }

    inline bool assert_numeric_type(type t)
    {
        switch (t)
        {
        case type::OBJECT:
        case type::METHOD:
            return false;
        default:
            return true;
        }
    }

    inline bool assert_integer_type(type t)
    {
        switch (t)
        {
        case type::FLOAT:
        case type::DOUBLE:
        case type::OBJECT:
        case type::METHOD:
            return false;
        default:
            return true;
        }
    }

    template <typename t1>
    struct common_type_of_type
    {
        template <typename t2>
        struct and_type
        {
            typedef std::common_type_t<t1, t2> type;
        };
    };
} // namespace

int virtual_machine::execute()
{
    PUN(std::uint64_t, instr, next_instruction.back());
    std::uint16_t dest = instr >> 48u & 0xffffull,
                  src1 = instr >> (sizeof(std::uint16_t) << 4u) & 0xffffull,
                  src2 = instr >> (sizeof(std::uint16_t) << 3u) & 0xffffull;
    std::uint8_t typeinfo = instr >> (sizeof(uint8_t) << 3u) & 0xffull,
                 opcode = 0xffull;
    ::type t1 = static_cast<::type>(typeinfo & 0b111),
           t2 = static_cast<::type>(typeinfo >> 3 & 0b111),
           td = std::max(t1, t2);

    switch (static_cast<OPCODE>(opcode))
    {
#define EXEC(condition, r1, r2, w, op)                                                                                                                                                                   \
    if ((condition) or not r1([this, src2, t2, dest, td](auto p1) { return r2([this, p1, dest, td](auto p2) { return w([](auto a, auto b) { return op; }, p1, p2, dest, td); }, src2, t2); }, src1, t1)) \
        return this->invalid_bytecode(instr);                                                                                                                                                            \
    break
#define PRIMITIVE_CONVERT(r2, w, op) EXEC(not assert_numeric_type(t1) or not assert_numeric_type(t2) or not assert_convertible_types(t1, t2), this->read_stack_primitive, r2, w, op)
#define PRIMITIVE_2(op) PRIMITIVE_CONVERT(this->read_stack_primitive, this->primitive_execute_writeback, op)
#define PRIMITIVE_IMM(op) PRIMITIVE_CONVERT(this->read_imm_primitive, this->primitive_execute_writeback, op)
#pragma region //primitives
    case OPCODE::ADD:
        PRIMITIVE_2(a + b);
    case OPCODE::SUB:
        PRIMITIVE_2(a - b);
    case OPCODE::MUL:
        PRIMITIVE_2(a * b);
    case OPCODE::DIV:
        PRIMITIVE_2(a / b);
    case OPCODE::ADDI:
        PRIMITIVE_IMM(a + b);
    case OPCODE::MULI:
        PRIMITIVE_IMM(a * b);
    case OPCODE::DIVI:
        PRIMITIVE_IMM(a / b);
    case OPCODE::NEG:
#pragma GCC diagnostic ignored "-Wunused-parameter"
        PRIMITIVE_2(-a);
#pragma GCC diagnostic pop
    case OPCODE::CAST:
#pragma GCC diagnostic ignored "-Wunused-parameter"
        PRIMITIVE_IMM(static_cast<decltype(b)>(a));
#pragma GCC diagnostic pop
    case OPCODE::LUI:
    {
        std::uint32_t imm = instr >> 32;
        if (typeinfo > 0b111)
            this->invalid_bytecode(instr);
        switch (td)
        {
        case ::type::INT:
            this->stack.write(dest, static_cast<std::int32_t>(imm));
            break;
        case ::type::LONG:
            this->stack.write(dest, static_cast<std::int64_t>((this->stack.read<std::int64_t>(dest) & 0xffffull) | (static_cast<std::uint64_t>(imm) << 32u)));
            break;
        case ::type::FLOAT:
            this->stack.write(dest, static_cast<float>(imm));
            break;
        case ::type::SHORT:
            this->stack.write(dest, static_cast<std::int16_t>(imm));
            break;
        case ::type::CHAR:
            this->stack.write(dest, static_cast<std::int8_t>(imm));
            break;
        case ::type::DOUBLE:
            this->stack.write(dest, static_cast<double>(imm));
            break;
        default:
            return this->invalid_bytecode(instr);
        }
        break;
    }
#pragma endregion
#pragma region //Integer ops
#define INTEGER_CONVERT(r2, w, op) EXEC(not assert_integer_type(t1) or not assert_integer_type(t2), this->read_stack_integer, r2, w, op)
#define INTEGER_2(op) INTEGER_CONVERT(this->read_stack_integer, this->integer_execute_writeback, op)
#define INTEGER_IMM(op) INTEGER_CONVERT(this->read_imm_integer, this->integer_execute_writeback, op)
    case OPCODE::AND:
        INTEGER_2(a & b);
    case OPCODE::OR:
        INTEGER_2(a | b);
    case OPCODE::XOR:
        INTEGER_2(a ^ b);
    case OPCODE::SLL:
        INTEGER_2(a << b);
    case OPCODE::SRL:
        if ((not assert_integer_type(t1) or not assert_integer_type(t2)) or not this->read_stack_integer([this, src2, t2, dest, td](auto p1) { return this->read_stack_integer([this, p1, dest, td](auto p2) { return this->integer_execute_writeback([](auto a, auto b) { return static_cast<std::make_signed_t<typename ::common_type_of_type<decltype(a)>::and_type<decltype(b)>::type>>(static_cast<std::make_unsigned_t<decltype(a)>>(a) >> static_cast<std::make_unsigned_t<decltype(b)>>(b)); }, p1, p2, dest, td); }, src2, t2); }, src1, t1))
            return this->invalid_bytecode(instr);
        break;
    case OPCODE::SRA:
        INTEGER_2(a >> b);
    case OPCODE::ANDI:
        INTEGER_2(a & b);
    case OPCODE::ORI:
        INTEGER_2(a | b);
    case OPCODE::XORI:
        INTEGER_2(a ^ b);
    case OPCODE::SLLI:
        INTEGER_2(a << b);
    // case OPCODE::SRLI:
    //     INTEGER_2(static_cast<std::make_signed_t<typename ::common_type_of_type<decltype(a)>::and_type<decltype(b)>::type>>(static_cast<std::make_unsigned_t<decltype(a)>>(a) >> static_cast<std::make_unsigned_t<decltype(b)>>(b)));
    case OPCODE::SRAI:
        INTEGER_2(a >> b);
#pragma endregion
#pragma region //branching
#define BRANCH_CONVERT(r2, w, op) EXEC(forward > 1 or not assert_numeric_type(t1) or not assert_numeric_type(t2) or not assert_convertible_types(t1, t2), this->read_stack_primitive, r2, w, op)
#define BRANCH_2(op) BRANCH_CONVERT(this->read_stack_primitive, this->branch, op)
#define BRANCH_IMM(op) BRANCH_CONVERT(this->read_imm_primitive, this->branch, op)
//Nasty hack to allow continued usage of INTERPRET
#define td forward
    case OPCODE::BGE:
    {
        signed char forward = typeinfo >> 6;
        BRANCH_2(a >= b);
    }
    case OPCODE::BLT:
    {
        signed char forward = typeinfo >> 6;
        BRANCH_2(a < b);
    }
    case OPCODE::BLE:
    {
        signed char forward = typeinfo >> 6;
        BRANCH_2(a <= b);
    }
    case OPCODE::BGT:
    {
        signed char forward = typeinfo >> 6;
        BRANCH_2(a > b);
    }
    // case OPCODE::BEQ:
    // {
    //     signed char forward = typeinfo >> 6;
    //     INTERPRET(forward > 1 or not((assert_numeric_type(t1) and assert_numeric_type(t2)) or (t1 == ::type::OBJECT and t2 == ::type::OBJECT)) or not assert_convertible_types(t1, t2), this->eq_decode_src2, [](auto a, auto b) { return a == b; });
    // }
    // case OPCODE::BNEQ:
    // {
    //     signed char forward = typeinfo >> 6;
    //     INTERPRET(forward > 1 or not((assert_numeric_type(t1) and assert_numeric_type(t2)) or (t1 == ::type::OBJECT and t2 == ::type::OBJECT)) or not assert_convertible_types(t1, t2), this->eq_decode_src2, [](auto a, auto b) { return a != b; });
    // }
    case OPCODE::BGEI:
    {
        signed char forward = typeinfo >> 6;
        BRANCH_IMM(a >= b);
    }
    case OPCODE::BLTI:
    {
        signed char forward = typeinfo >> 6;
        BRANCH_IMM(a < b);
    }
    case OPCODE::BLEI:
    {
        signed char forward = typeinfo >> 6;
        BRANCH_IMM(a <= b);
    }
    case OPCODE::BGTI:
    {
        signed char forward = typeinfo >> 6;
        BRANCH_IMM(a > b);
    }
    // case OPCODE::BEQI:
    // {
    //     signed char forward = typeinfo >> 6;
    //     INTERPRET(forward > 1 or not(assert_numeric_type(t1) and assert_numeric_type(t2) or t1 == ::type::OBJECT and t2 == ::type::OBJECT and src2 == 0) or not assert_convertible_types(t1, t2), this->eq_decode_imm, [](auto a, auto b) { return a == b; });
    // }
    // case OPCODE::BNEQI:
    // {
    //     signed char forward = typeinfo >> 6;
    //     INTERPRET(forward > 1 or not(assert_numeric_type(t1) and assert_numeric_type(t2) or t1 == ::type::OBJECT and t2 == ::type::OBJECT and src2 == 0) or not assert_convertible_types(t1, t2), this->eq_decode_imm, [](auto a, auto b) { return a != b; });
    // }
#undef td
#pragma endregion
    }
    this->next_instruction.back() += sizeof(std::uint64_t);
    return 0;
#undef INTERPRET
}