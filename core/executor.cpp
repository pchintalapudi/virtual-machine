#include "executor.h"

#include "../classes/datatypes.h"
#include "../classes/field_descriptor.h"
#include "arithmetic.h"
using namespace oops::core;

namespace {
template <typename raw_t>
using safe_type = std::conditional_t<(sizeof(raw_t) < sizeof(std::int32_t)),
                                     std::int32_t, raw_t>;
}

oops_wrapper_t executor::invoke(classes::clazz context, methods::method method,
                                const oops_wrapper_t *args, int nargs) {
  this->vm_stack.push_native_frame(context, method, args, nargs);
  int depth = 1;
  oops::instr_idx_t next_instruction = 0;
  const char *exception_message = nullptr;
  while (true) {
    // TODO validate next instruction
    instructions::instruction instr =
        this->vm_stack.current_frame().executing_method().read_instruction(
            next_instruction);
    typedef instructions::instruction::itype itype;
#define type_error(action, target, instr_type, wanted_type)                \
  "Failed to " #action " " #target " for instruction of type " #instr_type \
  " because " #target " is not a " #wanted_type "!!\n";
#define load_src(name, type, instr_type)                               \
  std::optional<type> maybe_##name =                                   \
      this->vm_stack.current_frame().checked_read<type>(instr.name()); \
  if (!maybe_##name) {                                                 \
    exception_message = type_error(decode, name, instr_type, type);    \
    goto exception;                                                    \
  }                                                                    \
  type name = *maybe_##name
#define writeback_dest(name, type, instr_type)                          \
  bool success =                                                        \
      this->vm_stack.current_frame().checked_write(instr.dest(), name); \
  if (!success) {                                                       \
    exception_message = type_error(write back, dest, instr_type, type); \
    goto exception;                                                     \
  }
    switch (instr.type_of()) {
#define numeric_op(type, instr_type, op)             \
  {                                                  \
    load_src(src1, type, instr_type);                \
    load_src(src2, type, instr_type);                \
    type result = arithmetic::op##_safe(src1, src2); \
    writeback_dest(result, type, instr_type);        \
    break;                                           \
  }
#define integer_types_numeric_op(caps, lower) \
  case itype::I##caps: {                      \
    numeric_op(std::int32_t, I##caps, lower); \
  }                                           \
  case itype::L##caps: {                      \
    numeric_op(std::int64_t, L##caps, lower); \
  }
#define all_types_numeric_op(caps, lower) \
  integer_types_numeric_op(caps, lower);  \
  case itype::F##caps: {                  \
    numeric_op(float, F##caps, lower);    \
  }                                       \
  case itype::D##caps: {                  \
    numeric_op(double, D##caps, lower);   \
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
#define load_imm(name, type, instr_type) type name = instr.imm24()
#define numeric_imm_op(type, instr_type, op)         \
  {                                                  \
    load_src(src1, type, instr_type);                \
    load_imm(src2, type, instr_type);                \
    type result = arithmetic::op##_safe(src1, src2); \
    writeback_dest(result, type, instr_type);        \
    break;                                           \
  }
#define integer_types_numeric_imm_op(caps, lower)    \
  case itype::I##caps##I: {                          \
    numeric_imm_op(std::int32_t, I##caps##I, lower); \
  }                                                  \
  case itype::L##caps##I: {                          \
    numeric_imm_op(std::int64_t, L##caps##I, lower); \
  }
#define all_types_numeric_imm_op(caps, lower)  \
  integer_types_numeric_imm_op(caps, lower);   \
  case itype::F##caps##I: {                    \
    numeric_imm_op(float, F##caps##I, lower);  \
  }                                            \
  case itype::D##caps##I: {                    \
    numeric_imm_op(double, D##caps##I, lower); \
  }
        all_types_numeric_imm_op(ADD, add);
        all_types_numeric_imm_op(MUL, mul);
      case itype::IDIVI: {
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
      case itype::LDIVI: {
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
      case itype::FDIVI: {
        load_src(src1, float, FDIVI);
        load_imm(src2, float, FDIVI);
        std::optional<float> result = arithmetic::div_safe(src1, src2);
        writeback_dest(*result, float, FDIVI);
        break;
      }
      case itype::DDIVI: {
        load_src(src1, double, DDIVI);
        load_imm(src2, double, DDIVI);
        std::optional<double> result = arithmetic::div_safe(src1, src2);
        writeback_dest(*result, double, DDIVI);
        break;
      }
      case itype::IMODI: {
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
      case itype::LMODI: {
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
      case itype::FMODI: {
        load_src(src1, float, FMODI);
        load_imm(src2, float, FMODI);
        std::optional<float> result = arithmetic::mod_safe(src1, src2);
        writeback_dest(*result, float, FMODI);
        break;
      }
      case itype::DMODI: {
        load_src(src1, double, DMODI);
        load_imm(src2, double, DMODI);
        std::optional<double> result = arithmetic::mod_safe(src1, src2);
        writeback_dest(*result, double, DMODI);
        break;
      }
      case itype::IRSUBI: {
        load_src(src1, std::int32_t, IRSUBI);
        load_imm(src2, std::int32_t, IRSUBI);
        std::int32_t result = arithmetic::sub_safe(src2, src1);
        writeback_dest(result, std::int32_t, IRSUBI);
        break;
      }
      case itype::LRSUBI: {
        load_src(src1, std::int64_t, LRSUBI);
        load_imm(src2, std::int64_t, LRSUBI);
        std::int64_t result = arithmetic::sub_safe(src2, src1);
        writeback_dest(result, std::int64_t, LRSUBI);
        break;
      }
      case itype::FRSUBI: {
        load_src(src1, float, FRSUBI);
        load_imm(src2, float, FRSUBI);
        float result = arithmetic::sub_safe(src2, src1);
        writeback_dest(result, float, FRSUBI);
        break;
      }
      case itype::DRSUBI: {
        load_src(src1, double, DRSUBI);
        load_imm(src2, double, DRSUBI);
        double result = arithmetic::sub_safe(src2, src1);
        writeback_dest(result, double, DRSUBI);
        break;
      }
      case itype::IRDIVI: {
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
      case itype::LRDIVI: {
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
      case itype::FRDIVI: {
        load_src(src1, float, FRDIVI);
        load_imm(src2, float, FRDIVI);
        std::optional<float> result = arithmetic::div_safe(src2, src1);
        writeback_dest(*result, float, FRDIVI);
        break;
      }
      case itype::DRDIVI: {
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
#define branch_op(cmp, type, instr_type, cmp_name)                            \
  {                                                                           \
    load_src(src1, type, instr_type);                                         \
    load_src(src2, type, instr_type);                                         \
    next_instruction = src1 cmp src2 ? instr.target() : next_instruction + 1; \
    continue;                                                                 \
  }
#define all_types_branch_op(cmp, caps, cmp_name)     \
  case itype::I##caps: {                             \
    branch_op(cmp, std::int32_t, I##caps, cmp_name); \
  }                                                  \
  case itype::L##caps: {                             \
    branch_op(cmp, std::int64_t, L##caps, cmp_name); \
  }                                                  \
  case itype::F##caps: {                             \
    branch_op(cmp, float, F##caps, cmp_name);        \
  }                                                  \
  case itype::D##caps: {                             \
    branch_op(cmp, double, D##caps, cmp_name);       \
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
#define branch_imm_op(cmp, type, instr_type, cmp_name)                        \
  {                                                                           \
    load_src(src1, type, instr_type);                                         \
    load_imm(src2, type, instr_type);                                         \
    next_instruction = src1 cmp src2 ? instr.target() : next_instruction + 1; \
    continue;                                                                 \
  }
#define all_types_branch_imm_op(cmp, caps, cmp_name)        \
  case itype::I##caps##I: {                                 \
    branch_imm_op(cmp, std::int32_t, I##caps##I, cmp_name); \
  }                                                         \
  case itype::L##caps##I: {                                 \
    branch_imm_op(cmp, std::int64_t, L##caps##I, cmp_name); \
  }                                                         \
  case itype::F##caps##I: {                                 \
    branch_imm_op(cmp, float, F##caps##I, cmp_name);        \
  }                                                         \
  case itype::D##caps##I: {                                 \
    branch_imm_op(cmp, double, D##caps##I, cmp_name);       \
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
#define cast_op(type1, type2, instr_type)           \
  {                                                 \
    load_src(src1, type1, instr_type);              \
    writeback_dest(src1, std::int64_t, instr_type); \
    break;                                          \
  }
#define all_cast_ops(l1, t1, l2, t2, l3, t3, l4, t4) \
  case itype::l1##TO##l2: {                          \
    cast_op(t1, t2, l1##TO##l2);                     \
  }                                                  \
  case itype::l1##TO##l3: {                          \
    cast_op(t1, t3, l1##TO##l3);                     \
  }                                                  \
  case itype::l1##TO##l4: {                          \
    cast_op(t1, t4, l1##TO##l4);                     \
  }
        all_cast_ops(I, std::int32_t, L, std::int64_t, F, float, D, double);
        all_cast_ops(L, std::int64_t, I, std::int32_t, F, float, D, double);
        all_cast_ops(F, float, I, std::int32_t, L, std::int64_t, D, double);
        all_cast_ops(D, double, I, std::int32_t, L, std::int64_t, F, float);
#define reinterpret_op(from_type, to_type, instr_type) \
  {                                                    \
    load_src(src1, from_type, instr_type);             \
    to_type result;                                    \
    memcpy(&result, &src1, sizeof(result));            \
    writeback_dest(result, to_type, instr_type);       \
    break;                                             \
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
      case itype::LCS: {
        auto maybe_str =
            this->vm_stack.current_frame().context_class().load_constant_string(
                instr.idx24());
        if (!maybe_str) {
          exception_message = "Invalid string index for instruction LCS!!\n";
          goto exception;
        }
        writeback_dest(maybe_str->to_base_object(), objects::base_object, LCS);
        break;
      }
#define npe(instr_type)                                                       \
  exception_message = "Null Pointer Exception - detected during " #instr_type \
                      " instruction!!\n";                                     \
  goto exception;
#define oob(instr_type)                                                        \
  exception_message =                                                          \
      "Index Out of Bounds - detected during " #instr_type " instruction!!\n"; \
  goto exception;
#define ald(type, initial)                                                  \
  case itype::initial##ALD: {                                               \
    load_src(src1, classes::base_object, initial##ALD);                     \
    if (src1.is_null()) {                                                   \
      npe(initial##ALD);                                                    \
    }                                                                       \
    if (!src1.is_array()) {                                                 \
      exception_message =                                                   \
          type_error(load type from array, src1, initial##ALD, array);      \
      goto exception;                                                       \
    }                                                                       \
    classes::array array = src1.as_array();                                 \
    load_src(src2, std::int32_t, initial##ALD);                             \
    if (static_cast<std::uint32_t>(array.length()) <=                       \
        static_cast<std::uint32_t>(src2)) {                                 \
      oob(initial##ALD);                                                    \
    }                                                                       \
    std::optional<type> result = array.get<type>(src2);                     \
    if (!result) {                                                          \
      exception_message =                                                   \
          "Incorrect array element type - wanted " #type " for " #initial   \
          "ALD"                                                             \
          " instruction!!\n";                                               \
      goto exception;                                                       \
    }                                                                       \
    bool success = this->vm_stack.current_frame().checked_write(            \
        instr.dest(), static_cast<safe_type<type>>(*result));               \
    if (!success) {                                                         \
      exception_message = type_error(write back, dest, initial##ALD, type); \
      goto exception;                                                       \
    }                                                                       \
    break;                                                                  \
  }
        ald(std::int8_t, C);
        ald(std::int16_t, S);
        ald(std::int32_t, I);
        ald(std::int64_t, L);
        ald(float, F);
        ald(double, D);
        ald(classes::base_object, R);
#define asr(type, initial)                                                \
  case itype::initial##ASR: {                                             \
    load_src(dest, classes::base_object, initial##ASR);                   \
    if (dest.is_null()) {                                                 \
      npe(initial##ASR);                                                  \
    }                                                                     \
    if (!dest.is_array()) {                                               \
      exception_message =                                                 \
          type_error(store type to array, src1, initial##ASR, array);     \
      goto exception;                                                     \
    }                                                                     \
    classes::array array = dest.as_array();                               \
    load_src(src2, std::int32_t, initial##ASR);                           \
    if (static_cast<std::uint32_t>(array.length()) <=                     \
        static_cast<std::uint32_t>(src2)) {                               \
      oob(initial##ASR);                                                  \
    }                                                                     \
    load_src(src1, safe_type<type>, initial##ASR);                        \
    bool succeeded = array.set(src2, static_cast<type>(src1));            \
    if (!succeeded) {                                                     \
      exception_message =                                                 \
          "Incorrect array element type - wanted " #type " for " #initial \
          "ASR"                                                           \
          " instruction!!\n";                                             \
      goto exception;                                                     \
    }                                                                     \
    break;                                                                \
  }
        asr(std::int8_t, C);
        asr(std::int16_t, S);
        asr(std::int32_t, I);
        asr(std::int64_t, L);
        asr(float, F);
        asr(double, D);
        asr(classes::base_object, R);
// TODO cache loaded class
// TODO cache field index
#define get_class_field_descriptor(instr_type, dt)                            \
  std::uint32_t idx24 = instr.idx24();                                        \
  classes::clazz context = this->vm_stack.current_frame().context_class();    \
  std::optional<classes::field_descriptor> maybe_descriptor =                 \
      context.get_field_descriptor(idx24);                                    \
  if (!maybe_descriptor) {                                                    \
    exception_message =                                                       \
        "Invalid field descriptor index for instruction " #instr_type "!!\n"; \
    goto exception;                                                           \
  }                                                                           \
  auto descriptor = *maybe_descriptor;                                        \
  if (std::holds_alternative<classes::string>(descriptor.field_index)) {      \
    if (std::holds_alternative<classes::string>(descriptor.clazz)) {          \
      std::optional<classes::clazz> loaded =                                  \
          this->bootstrap_classloader.load_class(                             \
              std::get<classes::string>(descriptor.clazz));                   \
      if (!loaded) {                                                          \
        exception_message =                                                   \
            "Class Load Exception - unable to load class for "                \
            "instruction " #instr_type "!!\n";                                \
        goto exception;                                                       \
      }                                                                       \
      descriptor.clazz = *loaded;                                             \
    }                                                                         \
    std::optional<std::uint32_t> class_field_index =                          \
        std::get<classes::clazz>(descriptor.clazz)                            \
            .reflect_class_field_index(                                       \
                std::get<classes::string>(descriptor.field_index),            \
                classes::datatype::dt);                                       \
    if (!class_field_index) {                                                 \
      exception_message =                                                     \
          "Field Definition Exception - unable to get class field for "       \
          "instruction " #instr_type "!!\n";                                  \
      goto exception;                                                         \
    }                                                                         \
    descriptor.field_index = *class_field_index;                              \
  }
#define cld(instr_type, type, dt)                                          \
  case itype::instr_type: {                                                \
    get_class_field_descriptor(instr_type, dt);                            \
    std::optional<type> result =                                           \
        std::get<classes::clazz>(descriptor.clazz)                         \
            .checked_read_static_memory<type>(                             \
                std::get<std::uint32_t>(descriptor.field_index));          \
    if (!result) {                                                         \
      exception_message = type_error(load, class field, instr_type, type); \
      goto exception;                                                      \
    }                                                                      \
    bool success = this->vm_stack.current_frame().checked_write(           \
        instr.dest(), static_cast<safe_type<type>>(*result));              \
    if (!success) {                                                        \
      exception_message = type_error(write back, dest, instr_type, type);  \
      goto exception;                                                      \
    }                                                                      \
    break;                                                                 \
  }
        cld(CCLD, std::int8_t, BYTE);
        cld(SCLD, std::int16_t, SHORT);
        cld(ICLD, std::int32_t, INT);
        cld(LCLD, std::int64_t, LONG);
        cld(FCLD, float, FLOAT);
        cld(DCLD, double, DOUBLE);
        cld(RCLD, classes::base_object, OBJECT);
#define csr(instr_type, type, dt)                                           \
  case itype::instr_type: {                                                 \
    get_class_field_descriptor(instr_type, dt);                             \
    load_src(src1, safe_type<type>, instr_type);                            \
    bool success = std::get<classes::clazz>(descriptor.clazz)               \
                       .checked_write_static_memory(                        \
                           std::get<std::uint32_t>(descriptor.field_index), \
                           static_cast<type>(src1));                        \
    if (!success) {                                                         \
      exception_message = type_error(store, class field, instr_type, type); \
      goto exception;                                                       \
    }                                                                       \
    break;                                                                  \
  }
        csr(CCSR, std::int8_t, BYTE);
        csr(SCSR, std::int16_t, SHORT);
        csr(ICSR, std::int32_t, INT);
        csr(LCSR, std::int64_t, LONG);
        csr(FCSR, float, FLOAT);
        csr(DCSR, double, DOUBLE);
        csr(RCSR, classes::base_object, OBJECT);
// TODO cache loaded class
// TODO cache field index
#define get_object_field_descriptor(instr_type, dt)                           \
  std::uint32_t idx24 = instr.idx24();                                        \
  classes::clazz context = this->vm_stack.current_frame().context_class();    \
  std::optional<classes::field_descriptor> maybe_descriptor =                 \
      context.get_field_descriptor(idx24);                                    \
  if (!maybe_descriptor) {                                                    \
    exception_message =                                                       \
        "Invalid field descriptor index for instruction " #instr_type "!!\n"; \
    goto exception;                                                           \
  }                                                                           \
  auto descriptor = *maybe_descriptor;                                        \
  if (std::holds_alternative<classes::string>(descriptor.field_index)) {      \
    if (std::holds_alternative<classes::string>(descriptor.clazz)) {          \
      std::optional<classes::clazz> loaded =                                  \
          this->bootstrap_classloader.load_class(                             \
              std::get<classes::string>(descriptor.clazz));                   \
      if (!loaded) {                                                          \
        exception_message =                                                   \
            "Class Load Exception - unable to load class for "                \
            "instruction " #instr_type "!!\n";                                \
        goto exception;                                                       \
      }                                                                       \
      descriptor.clazz = *loaded;                                             \
    }                                                                         \
    std::optional<std::uint32_t> class_field_index =                          \
        std::get<classes::clazz>(descriptor.clazz)                            \
            .reflect_object_field_index(                                      \
                std::get<classes::string>(descriptor.field_index),            \
                classes::datatype::dt);                                       \
    if (!class_field_index) {                                                 \
      exception_message =                                                     \
          "Field Definition Exception - unable to get object field for "      \
          "instruction " #instr_type "!!\n";                                  \
      goto exception;                                                         \
    }                                                                         \
    descriptor.field_index = *class_field_index;                              \
  }
// TODO check object class assignable to field descriptor class
#define old(instr_type, type, dt)                                         \
  case itype::instr_type: {                                               \
    get_object_field_descriptor(instr_type, dt);                          \
    load_src(src1, classes::base_object, instr_type);                     \
    if (src1.is_null()) {                                                 \
      npe(instr_type);                                                    \
    }                                                                     \
    if (src1.is_array()) {                                                \
      exception_message =                                                 \
          type_error(load type from object, src1, instr_type, object);    \
      goto exception;                                                     \
    }                                                                     \
    classes::object obj = src1.as_object();                               \
    std::optional<type> result =                                          \
        obj.read<type>(std::get<std::uint32_t>(descriptor.field_index));  \
    if (!result) {                                                        \
      exception_message = "Incorrect object field type - wanted " #type   \
                          " for " #instr_type " instruction!!\n";         \
      goto exception;                                                     \
    }                                                                     \
    bool success = this->vm_stack.current_frame().checked_write(          \
        instr.dest(), static_cast<safe_type<type>>(*result));             \
    if (!success) {                                                       \
      exception_message = type_error(write back, dest, instr_type, type); \
      goto exception;                                                     \
    }                                                                     \
    break;                                                                \
  }
        old(COLD, std::int8_t, BYTE);
        old(SOLD, std::int16_t, SHORT);
        old(IOLD, std::int32_t, INT);
        old(LOLD, std::int64_t, LONG);
        old(FOLD, float, FLOAT);
        old(DOLD, double, DOUBLE);
        old(ROLD, classes::base_object, OBJECT);
// TODO check object class assignable to field descriptor class
#define osr(instr_type, type, dt)                                             \
  case itype::instr_type: {                                                   \
    get_object_field_descriptor(instr_type, dt);                              \
    load_src(dest, classes::base_object, instr_type);                         \
    if (dest.is_null()) {                                                     \
      npe(instr_type);                                                        \
    }                                                                         \
    if (dest.is_array()) {                                                    \
      exception_message =                                                     \
          type_error(load type from object, dest, instr_type, object);        \
      goto exception;                                                         \
    }                                                                         \
    classes::object obj = dest.as_object();                                   \
    load_src(src1, safe_type<type>, instr_type);                              \
    bool success = obj.write(std::get<std::uint32_t>(descriptor.field_index), \
                             static_cast<type>(src1));                        \
    if (!success) {                                                           \
      exception_message = "Incorrect object field type - wanted " #type       \
                          " for " #instr_type " instruction!!\n";             \
      goto exception;                                                         \
    }                                                                         \
    break;                                                                    \
  }
        osr(COSR, std::int8_t, BYTE);
        osr(SOSR, std::int16_t, SHORT);
        osr(IOSR, std::int32_t, INT);
        osr(LOSR, std::int64_t, LONG);
        osr(FOSR, float, FLOAT);
        osr(DOSR, double, DOUBLE);
        osr(ROSR, classes::base_object, OBJECT);
      case itype::ONEW: {
        auto descriptor =
            this->vm_stack.current_frame().context_class().get_class_descriptor(
                instr.idx24());
        if (!descriptor) {
          exception_message =
              "Invalid class descriptor index for instruction ONEW!!\n";
          goto exception;
        }
        if (std::holds_alternative<classes::string>(*descriptor)) {
          auto cls = this->bootstrap_classloader.load_class(
              std::get<classes::string>(*descriptor));
          if (!cls) {
            exception_message = "Unable to load class for instruction ONEW!!\n";
            goto exception;
          }
          *descriptor = *cls;
        }
        auto obj = this->vm_heap->allocate_object(
            std::get<classes::clazz>(*descriptor));
        if (!obj) {
          exception_message = "Out of Memory Error!!\n";
          goto exception;
        }
        writeback_dest(obj->to_base_object(), classes::base_object, ONEW);
        break;
      }
#define anew(instr_type, dt)                                                   \
  case itype::instr_type: {                                                    \
    load_src(src1, std::int32_t, instr_type);                                  \
    auto array = this->vm_heap->allocate_array(classes::datatype::dt, src1);   \
    if (!array) {                                                              \
      exception_message = "Out of Memory Error!!\n";                           \
      goto exception;                                                          \
    }                                                                          \
    writeback_dest(array->to_base_object(), classes::base_object, instr_type); \
    break;                                                                     \
  }
        anew(CANEW, BYTE);
        anew(SANEW, SHORT);
        anew(IANEW, INT);
        anew(LANEW, LONG);
        anew(FANEW, FLOAT);
        anew(DANEW, DOUBLE);
        anew(RANEW, OBJECT);
      case itype::IOF: {
        auto descriptor =
            this->vm_stack.current_frame().context_class().get_class_descriptor(
                instr.idx24());
        if (!descriptor) {
          exception_message =
              "Invalid class descriptor index for instruction ONEW!!\n";
          goto exception;
        }
        if (std::holds_alternative<classes::string>(*descriptor)) {
          auto cls = this->bootstrap_classloader.load_class(
              std::get<classes::string>(*descriptor));
          if (!cls) {
            exception_message = "Unable to load class for instruction ONEW!!\n";
            goto exception;
          }
          *descriptor = *cls;
        }
        load_src(src1, classes::base_object, IOF);
        if (src1.is_null()) {
          writeback_dest(0, std::int32_t, IOF);
          break;
        }
        if (src1.is_array()) {
          // TODO figure out array classes here
          writeback_dest(0, std::int32_t, IOF);
          break;
        }
        writeback_dest(this->instanceof_table.is_superclass(
                           std::get<classes::clazz>(*descriptor),
                           src1.as_object().get_class()),
                       std::int32_t, IOF);
        break;
      }
#define ret(instr_type, real_type, dt, as)                                 \
  case itype::instr_type: {                                                \
    load_src(src1, safe_type<real_type>, instr_type);                      \
    if (!--depth) {                                                        \
      this->vm_stack.pop_frame();                                          \
      oops_wrapper_t ret;                                                  \
      ret.type = oops_wrapped_type_t::OOPS_##dt;                           \
      ret.as_##as = src1;                                                  \
      return ret;                                                          \
    }                                                                      \
    instr_idx_t next_instr =                                               \
        this->vm_stack.current_frame().get_return_address();               \
    stack_idx_t dest = this->vm_stack.current_frame().get_return_offset(); \
    this->vm_stack.pop_frame();                                            \
    next_instruction = next_instr;                                         \
    bool success = this->vm_stack.current_frame().checked_write(           \
        dest,                                                              \
        static_cast<safe_type<real_type>>(static_cast<real_type>(src1)));  \
    if (!success) {                                                        \
      exception_message = type_error(write back, dest, instr_type, type);  \
      goto exception;                                                      \
    }                                                                      \
    break;                                                                 \
  }
        ret(CRET, std::int8_t, BYTE, byte);
        ret(SRET, std::int16_t, SHORT, short);
        ret(IRET, std::int32_t, INT, int);
        ret(LRET, std::int64_t, LONG, long);
        ret(FRET, float, FLOAT, float);
        ret(DRET, double, DOUBLE, double);
      case itype::RRET: {
        load_src(src1, classes::base_object, RRET);
        if (!--depth) {
          this->vm_stack.pop_frame();
          oops_wrapper_t ret;
          ret.type = oops_wrapped_type_t::OOPS_OBJECT;
          ret.as_object = {src1.get_raw()};
          return ret;
        }
        instr_idx_t next_instr =
            this->vm_stack.current_frame().get_return_address();
        stack_idx_t dest = this->vm_stack.current_frame().get_return_offset();
        this->vm_stack.pop_frame();
        next_instruction = next_instr;
        bool success = this->vm_stack.current_frame().checked_write(dest, src1);
        if (!success) {
          exception_message =
              type_error(write back, dest, RRET, classes::base_object);
          goto exception;
        }
        break;
      }
      case itype::NRET: {
        if (!--depth) {
          this->vm_stack.pop_frame();
          oops_wrapper_t ret;
          ret.type = oops_wrapped_type_t::OOPS_VOID;
          return ret;
        }
        instr_idx_t next_instr =
            this->vm_stack.current_frame().get_return_address();
        this->vm_stack.pop_frame();
        next_instruction = next_instr;
        break;
      }
      case itype::SCALL: {
        auto descriptor = this->vm_stack.current_frame()
                            .context_class()
                            .get_static_method_descriptor(instr.idx24());
        if (!descriptor) {
          exception_message =
              "Invalid method descriptor for instruction SCALL!!\n";
          goto exception;
        }
        if (std::holds_alternative<classes::string>(descriptor->clazz)) {
          auto cls = this->bootstrap_classloader.load_class(
              std::get<classes::string>(descriptor->clazz));
          if (!cls) {
            exception_message = "Unable to load class for instruction SCALL!!\n";
            goto exception;
          }
          descriptor->clazz = *cls;
        }
        if (std::holds_alternative<classes::string>(descriptor->static_method)) {
            auto method = std::get<classes::clazz>(descriptor->clazz).reflect_static_method(std::get<classes::string>(descriptor->static_method));
            if (!method) {
                exception_message = "Could not find static method in class for instruction SCALL!!\n";
                goto exception;
            }
            descriptor->static_method = *method;
        }
        auto cls = std::get<classes::clazz>(descriptor->clazz);
        auto method = std::get<methods::method>(descriptor->static_method);
        this->vm_stack.push_frame(cls, method, this->vm_stack.current_frame().executing_method().get_args_for_called_instruction(next_instruction, method.arg_count()), instr.dest(), next_instruction + method.arg_count() / 8 + 1);
        next_instruction = 0;
        continue;
      }
      case itype::VCALL: {
        auto descriptor = this->vm_stack.current_frame()
                            .context_class()
                            .get_virtual_method_descriptor(instr.idx24());
        if (!descriptor) {
          exception_message =
              "Invalid method descriptor for instruction VCALL!!\n";
          goto exception;
        }
        if (std::holds_alternative<classes::string>(descriptor->clazz)) {
          auto cls = this->bootstrap_classloader.load_class(
              std::get<classes::string>(descriptor->clazz));
          if (!cls) {
            exception_message = "Unable to load class for instruction VCALL!!\n";
            goto exception;
          }
          descriptor->clazz = *cls;
        }
        if (std::holds_alternative<classes::string>(descriptor->virtual_method_index)) {
            auto idx = std::get<classes::clazz>(descriptor->clazz).reflect_virtual_method_index(std::get<classes::string>(descriptor->virtual_method_index));
            if (!idx) {
                exception_message = "Could not find virtual method index in class for instruction VCALL!!\n";
                goto exception;
            }
            descriptor->virtual_method_index = *idx;
        }
        load_src(src1, classes::base_object, VCALL);
        if (src1.is_null()) {
            npe(VCALL);
        }
        auto cls = std::get<classes::clazz>(descriptor->clazz);
        methods::method method(nullptr);
        if (src1.is_array()) {
            //TODO assert object class is array class and get method from there
            exception_message = "Failed to lookup virtual method by index for array for instruction VCALL!!\n";
            goto exception;
        } else {
            //TODO assert inheritance relationship
            auto mtd = src1.as_object().get_class().lookup_virtual_method_direct(std::get<std::uint32_t>(descriptor->virtual_method_index));
            if (!mtd) {
                exception_message = "Failed to lookup virtual method by index for instruction VCALL!!\n";
                goto exception;
            }
            method = *mtd;
        }
        this->vm_stack.push_frame(cls, method, this->vm_stack.current_frame().executing_method().get_args_for_called_instruction(next_instruction, method.arg_count()), instr.dest(), next_instruction + method.arg_count() / 8 + 1);
        next_instruction = 0;
        continue;
      }
      case itype::DCALL: {
        auto descriptor = this->vm_stack.current_frame()
                            .context_class()
                            .get_dynamic_method_descriptor(instr.idx24());
        if (!descriptor) {
          exception_message =
              "Invalid method descriptor for instruction DCALL!!\n";
          goto exception;
        }
        load_src(src1, classes::base_object, DCALL);
        auto cls = src1.as_object().get_class();
        auto method = cls.reflect_dynamic_method(descriptor->dynamic_method_name);
        if (!method) {
            exception_message = "Could not find static method in class for instruction DCALL!!\n";
            goto exception;
        }
        this->vm_stack.push_frame(cls, *method, this->vm_stack.current_frame().executing_method().get_args_for_called_instruction(next_instruction, method->arg_count()), instr.dest(), next_instruction + method->arg_count() / 8 + 1);
        next_instruction = 0;
        continue;
      }
      case itype::EXC:
      exception : {
        // TODO throw exception
      } break;
    }
    next_instruction++;
    if (exception_message != nullptr) {
      exception_message = "Forgot a goto exception somewhere!!\n";
      goto exception;
    }
  }
}

// Copied verbatim from https://stackoverflow.com/a/42927051
static_assert(std::numeric_limits<double>::is_iec559,
              "Please use IEEE754, you weirdo");