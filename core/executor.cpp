#include "executor.h"
#include "arithmetic.h"
using namespace oops::core;

namespace {
    template<typename raw_t>
    using safe_type = std::conditional_t<(sizeof(raw_t) < sizeof(std::int32_t)), std::int32_t, raw_t>;
}

oops_wrapper_t executor::invoke(methods::method* method, const oops_wrapper_t* args, int nargs) {
    this->vm_stack.push_frame(method);
    //TODO push args into frame
    int depth = 1;
    oops::instr_idx_t next_instruction = 0;
    const char* exception_message = nullptr;
    while (depth) {
        //TODO validate next instruction
        instructions::instruction instr = method->read_instruction(next_instruction);
        typedef instructions::instruction::itype itype;
        #define type_error(action, target, instr_type, wanted_type) "Failed to " #action " " #target " for instruction of type " #instr_type " because " #target " is not a " #wanted_type "!!\n";
        #define load_src(name, type, instr_type) \
            std::optional<type> maybe_##name = this->vm_stack.current_frame().checked_read<type>(instr.name());\
            if (!maybe_##name) {\
                exception_message = type_error(decode, name, instr_type, type);\
                goto exception;\
            }\
            type name = *maybe_##name
        #define writeback_dest(name, type, instr_type) \
            bool success = this->vm_stack.current_frame().checked_write(instr.dest(), name);\
            if (!success) {\
                exception_message = type_error(write back, dest, instr_type, type);\
                goto exception;\
            }
        switch (instr.type_of()) {
            #define numeric_op(type, instr_type, op) \
            {\
                load_src(src1, type, instr_type);\
                load_src(src2, type, instr_type);\
                type result = arithmetic::op##_safe(src1, src2);\
                writeback_dest(result, type, instr_type);\
                break;\
            }
            #define integer_types_numeric_op(caps, lower)\
            case itype::I##caps: {\
                numeric_op(std::int32_t, I##caps, lower);\
            }\
            case itype::L##caps: {\
                numeric_op(std::int64_t, L##caps, lower);\
            }
            #define all_types_numeric_op(caps, lower)\
            integer_types_numeric_op(caps, lower);\
            case itype::F##caps: {\
                numeric_op(float, F##caps, lower);\
            }\
            case itype::D##caps: {\
                numeric_op(double, D##caps, lower);\
            }
            all_types_numeric_op(ADD, add);
            all_types_numeric_op(SUB, sub);
            all_types_numeric_op(MUL, mul);
            case itype::IDIV: {
                load_src(src1, std::int32_t, IDIV);
                load_src(src2, std::int32_t, IDIV);
                std::optional<std::int32_t> result = arithmetic::div_safe(src1, src2);
                if (!result) {
                    exception_message = "Attempted to perform an int division by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int32_t, IDIV);
                break;
            }
            case itype::LDIV: {
                load_src(src1, std::int64_t, LDIV);
                load_src(src2, std::int64_t, LDIV);
                std::optional<std::int64_t> result = arithmetic::div_safe(src1, src2);
                if (!result) {
                    exception_message = "Attempted to perform a long division by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int64_t, LDIV);
                break;
            }
            case itype::FDIV: {
                load_src(src1, float, FDIV);
                load_src(src2, float, FDIV);
                std::optional<float> result = arithmetic::div_safe(src1, src2);
                writeback_dest(*result, float, FDIV);
                break;
            }
            case itype::DDIV: {
                load_src(src1, double, DDIV);
                load_src(src2, double, DDIV);
                std::optional<double> result = arithmetic::div_safe(src1, src2);
                writeback_dest(*result, double, DDIV);
                break;
            }
            case itype::IMOD: {
                load_src(src1, std::int32_t, IMOD);
                load_src(src2, std::int32_t, IMOD);
                std::optional<std::int32_t> result = arithmetic::mod_safe(src1, src2);
                if (!result) {
                    exception_message = "Attempted to perform an int mod by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int32_t, IMOD);
                break;
            }
            case itype::LMOD: {
                load_src(src1, std::int64_t, LMOD);
                load_src(src2, std::int64_t, LMOD);
                std::optional<std::int64_t> result = arithmetic::mod_safe(src1, src2);
                if (!result) {
                    exception_message = "Attempted to perform a long mod by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int64_t, LMOD);
                break;
            }
            case itype::FMOD: {
                load_src(src1, float, FMOD);
                load_src(src2, float, FMOD);
                std::optional<float> result = arithmetic::mod_safe(src1, src2);
                writeback_dest(*result, float, FMOD);
                break;
            }
            case itype::DMOD: {
                load_src(src1, double, DMOD);
                load_src(src2, double, DMOD);
                std::optional<double> result = arithmetic::mod_safe(src1, src2);
                writeback_dest(*result, double, DMOD);
                break;
            }
            integer_types_numeric_op(AND, and);
            integer_types_numeric_op(OR, or);
            integer_types_numeric_op(XOR, xor);
            integer_types_numeric_op(SLL, sll);
            integer_types_numeric_op(SRL, srl);
            integer_types_numeric_op(SRA, sra);
            #define load_imm(name, type, instr_type) \
            type name = instr.imm24()
            #define numeric_imm_op(type, instr_type, op) \
            {\
                load_src(src1, type, instr_type);\
                load_imm(src2, type, instr_type);\
                type result = arithmetic::op##_safe(src1, src2);\
                writeback_dest(result, type, instr_type);\
                break;\
            }
            #define integer_types_numeric_imm_op(caps, lower)\
            case itype::I##caps##I: {\
                numeric_imm_op(std::int32_t, I##caps##I, lower);\
            }\
            case itype::L##caps##I: {\
                numeric_imm_op(std::int64_t, L##caps##I, lower);\
            }
            #define all_types_numeric_imm_op(caps, lower)\
            integer_types_numeric_imm_op(caps, lower);\
            case itype::F##caps##I: {\
                numeric_imm_op(float, F##caps##I, lower);\
            }\
            case itype::D##caps##I: {\
                numeric_imm_op(double, D##caps##I, lower);\
            }
            all_types_numeric_imm_op(ADD, add);
            all_types_numeric_imm_op(MUL, mul);
            case itype::IDIVI:{
                load_src(src1, std::int32_t, IDIVI);
                load_imm(src2, std::int32_t, IDIVI);
                std::optional<std::int32_t> result = arithmetic::div_safe(src1, src2);
                if (!result) {
                    exception_message = "Attempted to perform an int division by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int32_t, IDIVI);
                break;
            }
            case itype::LDIVI:{
                load_src(src1, std::int64_t, LDIVI);
                load_imm(src2, std::int64_t, LDIVI);
                std::optional<std::int64_t> result = arithmetic::div_safe(src1, src2);
                if (!result) {
                    exception_message = "Attempted to perform a long division by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int64_t, LDIVI);
                break;
            }
            case itype::FDIVI:{
                load_src(src1, float, FDIVI);
                load_imm(src2, float, FDIVI);
                std::optional<float> result = arithmetic::div_safe(src1, src2);
                writeback_dest(*result, float, FDIVI);
                break;
            }
            case itype::DDIVI:{
                load_src(src1, double, DDIVI);
                load_imm(src2, double, DDIVI);
                std::optional<double> result = arithmetic::div_safe(src1, src2);
                writeback_dest(*result, double, DDIVI);
                break;
            }
            case itype::IMODI:{
                load_src(src1, std::int32_t, IMODI);
                load_imm(src2, std::int32_t, IMODI);
                std::optional<std::int32_t> result = arithmetic::mod_safe(src1, src2);
                if (!result) {
                    exception_message = "Attempted to perform an int mod by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int32_t, IMODI);
                break;
            }
            case itype::LMODI:{
                load_src(src1, std::int64_t, LMODI);
                load_imm(src2, std::int64_t, LMODI);
                std::optional<std::int64_t> result = arithmetic::mod_safe(src1, src2);
                if (!result) {
                    exception_message = "Attempted to perform a long mod by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int64_t, LMODI);
                break;
            }
            case itype::FMODI:{
                load_src(src1, float, FMODI);
                load_imm(src2, float, FMODI);
                std::optional<float> result = arithmetic::mod_safe(src1, src2);
                writeback_dest(*result, float, FMODI);
                break;
            }
            case itype::DMODI:{
                load_src(src1, double, DMODI);
                load_imm(src2, double, DMODI);
                std::optional<double> result = arithmetic::mod_safe(src1, src2);
                writeback_dest(*result, double, DMODI);
                break;
            }
            case itype::IRSUBI:{
                load_src(src1, std::int32_t, IRSUBI);
                load_imm(src2, std::int32_t, IRSUBI);
                std::int32_t result = arithmetic::sub_safe(src2, src1);
                writeback_dest(result, std::int32_t, IRSUBI);
                break;
            }
            case itype::LRSUBI:{
                load_src(src1, std::int64_t, LRSUBI);
                load_imm(src2, std::int64_t, LRSUBI);
                std::int64_t result = arithmetic::sub_safe(src2, src1);
                writeback_dest(result, std::int64_t, LRSUBI);
                break;
            }
            case itype::FRSUBI:{
                load_src(src1, float, FRSUBI);
                load_imm(src2, float, FRSUBI);
                float result = arithmetic::sub_safe(src2, src1);
                writeback_dest(result, float, FRSUBI);
                break;
            }
            case itype::DRSUBI:{
                load_src(src1, double, DRSUBI);
                load_imm(src2, double, DRSUBI);
                double result = arithmetic::sub_safe(src2, src1);
                writeback_dest(result, double, DRSUBI);
                break;
            }
            case itype::IRDIVI:{
                load_src(src1, std::int32_t, IRDIVI);
                load_imm(src2, std::int32_t, IRDIVI);
                std::optional<std::int32_t> result = arithmetic::div_safe(src2, src1);
                if (!result) {
                    exception_message = "Attempted to perform an int division by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int32_t, IRDIVI);
                break;
            }
            case itype::LRDIVI:{
                load_src(src1, std::int64_t, LRDIVI);
                load_imm(src2, std::int64_t, LRDIVI);
                std::optional<std::int64_t> result = arithmetic::div_safe(src2, src1);
                if (!result) {
                    exception_message = "Attempted to perform a long division by 0!!\n";
                    goto exception;
                }
                writeback_dest(*result, std::int64_t, LRDIVI);
                break;
            }
            case itype::FRDIVI:{
                load_src(src1, float, FRDIVI);
                load_imm(src2, float, FRDIVI);
                std::optional<float> result = arithmetic::div_safe(src2, src1);
                writeback_dest(*result, float, FRDIVI);
                break;
            }
            case itype::DRDIVI:{
                load_src(src1, double, DRDIVI);
                load_imm(src2, double, DRDIVI);
                std::optional<double> result = arithmetic::div_safe(src2, src1);
                writeback_dest(*result, double, DRDIVI);
                break;
            }
            integer_types_numeric_imm_op(AND, and);
            integer_types_numeric_imm_op(OR, or);
            integer_types_numeric_imm_op(XOR, xor);
            integer_types_numeric_imm_op(SLL, sll);
            integer_types_numeric_imm_op(SRL, srl);
            integer_types_numeric_imm_op(SRA, sra);
            #define branch_op(cmp, type, instr_type, cmp_name)\
            {\
                load_src(src1, type, instr_type);\
                load_src(src2, type, instr_type);\
                next_instruction = src1 cmp src2 ? instr.target() : next_instruction + 1;\
                continue;\
            }
            #define all_types_branch_op(cmp, caps, cmp_name)\
            case itype::I##caps: {\
                branch_op(cmp, std::int32_t, I##caps, cmp_name);\
            }\
            case itype::L##caps: {\
                branch_op(cmp, std::int64_t, L##caps, cmp_name);\
            }\
            case itype::F##caps: {\
                branch_op(cmp, float, F##caps, cmp_name);\
            }\
            case itype::D##caps: {\
                branch_op(cmp, double, D##caps, cmp_name);\
            }
            all_types_branch_op(<, BLT, blt);
            all_types_branch_op(<=, BLE, ble);
            all_types_branch_op(==, BEQ, beq);
            case itype::RBEQ: {
                load_src(src1, classes::base_object, RBEQ);
                load_src(src2, classes::base_object, RBEQ);
                next_instruction = src1 == src2 ? instr.target() : next_instruction + 1;
                continue;
            }
            #define branch_imm_op(cmp, type, instr_type, cmp_name)\
            {\
                load_src(src1, type, instr_type);\
                load_imm(src2, type, instr_type);\
                next_instruction = src1 cmp src2 ? instr.target() : next_instruction + 1;\
                continue;\
            }
            #define all_types_branch_imm_op(cmp, caps, cmp_name)\
            case itype::I##caps##I: {\
                branch_imm_op(cmp, std::int32_t, I##caps##I, cmp_name);\
            }\
            case itype::L##caps##I: {\
                branch_imm_op(cmp, std::int64_t, L##caps##I, cmp_name);\
            }\
            case itype::F##caps##I: {\
                branch_imm_op(cmp, float, F##caps##I, cmp_name);\
            }\
            case itype::D##caps##I: {\
                branch_imm_op(cmp, double, D##caps##I, cmp_name);\
            }
            all_types_branch_imm_op(<, BLT, blti);
            all_types_branch_imm_op(<=, BLE, blei);
            all_types_branch_imm_op(==, BEQ, beqi);
            case itype::BNULL: {
                load_src(src1, classes::base_object, BNULL);
                next_instruction = src1.is_null() ? instr.dest() : next_instruction + 1;
                continue;
            }
            case itype::BU: {
                next_instruction = instr.dest();
                continue;
            }
            #define cast_op(type1, type2, instr_type)\
            {\
                load_src(src1, type1, instr_type);\
                writeback_dest(src1, std::int64_t, instr_type);\
                break;\
            }
            #define all_cast_ops(l1, t1, l2, t2, l3, t3, l4, t4)\
            case itype::l1##TO##l2:\
            {\
                cast_op(t1, t2, l1##TO##l2);\
            }\
            case itype::l1##TO##l3:\
            {\
                cast_op(t1, t3, l1##TO##l3);\
            }\
            case itype::l1##TO##l4:\
            {\
                cast_op(t1, t4, l1##TO##l4);\
            }
            all_cast_ops(I, std::int32_t, L, std::int64_t, F, float, D, double);
            all_cast_ops(L, std::int64_t, I, std::int32_t, F, float, D, double);
            all_cast_ops(F, float, I, std::int32_t, L, std::int64_t, D, double);
            all_cast_ops(D, double, I, std::int32_t, L, std::int64_t, F, float);
            #define reinterpret_op(from_type, to_type, instr_type)\
            {\
                load_src(src1, from_type, instr_type);\
                to_type result;\
                memcpy(&result, &src1, sizeof(result));\
                writeback_dest(result, to_type, instr_type);\
                break;\
            }
            case itype::IASF: {
                reinterpret_op(std::int32_t, float, IASF);
            }
            case itype::LASD: {
                reinterpret_op(std::int64_t, double, LASD);
            }
            case itype::FASI: {
                reinterpret_op(float, std::int32_t, FASI);
            }
            case itype::DASL: {
                reinterpret_op(double, std::int64_t, DASL);
            }
            case itype::LUI: {
                std::int64_t imm = instr.imm40();
                writeback_dest(imm, std::int64_t, LUI);
                break;
            }
            case itype::LDI: {
                std::int32_t imm = instr.imm32();
                writeback_dest(imm, std::int32_t, LDI);
                break;
            }
            case itype::LNUL: {
                writeback_dest(classes::base_object(nullptr), object, LNUL);
                break;
            }
            #define npe(instr_type) exception_message = "Null Pointer Exception - detected during " #instr_type " instruction!!\n";goto exception;
            #define oob(instr_type) exception_message = "Index Out of Bounds - detected during " #instr_type " instruction!!\n";goto exception;
            #define ald(type, initial)\
            case itype::initial##ALD: {\
                load_src(src1, classes::base_object, initial##ALD);\
                if (src1.is_null()) {\
                    npe(initial##ALD);\
                }\
                if (!src1.is_array()) {\
                    exception_message = type_error(load type from array, src1, initial##ALD, array);\
                    goto exception;\
                }\
                classes::array array = src1.as_array();\
                load_src(src2, std::int32_t, initial##ALD);\
                if (static_cast<std::uint32_t>(array.length()) <= static_cast<std::uint32_t>(src2)) {\
                    oob(initial##ALD);\
                }\
                std::optional<type> result = array.get<type>(src2);\
                if (!result) {\
                    exception_message = "Incorrect array element type - wanted " #type " for " #initial "ALD" " instruction!!\n";\
                    goto exception;\
                }\
                bool success = this->vm_stack.current_frame().checked_write(instr.dest(), static_cast<safe_type<type>>(*result));\
                if (!success) {\
                    exception_message = type_error(write back, dest, initial##ALD, type);\
                    goto exception;\
                }\
                break;\
            }
            ald(std::int8_t, C);
            ald(std::int16_t, S);
            ald(std::int32_t, I);
            ald(std::int64_t, L);
            ald(float, F);
            ald(double, D);
            ald(classes::base_object, R);
            #define asr(type, initial)\
            case itype::initial##ASR: {\
                load_src(dest, classes::base_object, initial##ASR);\
                if (dest.is_null()) {\
                    npe(initial##ASR);\
                }\
                if (!dest.is_array()) {\
                    exception_message = type_error(store type to array, src1, initial##ASR, array);\
                    goto exception;\
                }\
                classes::array array = dest.as_array();\
                load_src(src2, std::int32_t, initial##ASR);\
                if (static_cast<std::uint32_t>(array.length()) <= static_cast<std::uint32_t>(src2)) {\
                    oob(initial##ASR);\
                }\
                load_src(src1, type, initial##ASR);\
                bool succeeded = array.set(src2, src1);\
                if (!succeeded) {\
                    exception_message = "Incorrect array element type - wanted " #type " for " #initial "ASR" " instruction!!\n";\
                    goto exception;\
                }\
                break;\
            }
            asr(std::int8_t, C);
            asr(std::int16_t, S);
            asr(std::int32_t, I);
            asr(std::int64_t, L);
            asr(float, F);
            asr(double, D);
            asr(classes::base_object, R);

            case itype::EXC:
            exception: {
                //TODO throw exception
            }
            break;
        }
        next_instruction++;
    }
}

//Copied verbatim from https://stackoverflow.com/a/42927051
static_assert(std::numeric_limits<double>::is_iec559, "Please use IEEE754, you weirdo");