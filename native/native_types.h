#ifndef OOPS_NATIVE_NATIVE_TYPES_H
#define OOPS_NATIVE_NATIVE_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum oops_wrapped_type_t {
  OOPS_BYTE,
  OOPS_SHORT,
  OOPS_INT,
  OOPS_LONG,
  OOPS_FLOAT,
  OOPS_DOUBLE,
  OOPS_OBJECT,
  OOPS_VOID,
  OOPS_THROWN_EXCEPTION
};

struct oops_object_t {
  void *object;
};

struct oops_wrapper_t {
  oops_wrapped_type_t type;
  union {
    int8_t as_byte;
    int16_t as_short;
    int32_t as_int;
    int64_t as_long;
    float as_float;
    double as_double;
    oops_object_t as_object;
    oops_object_t as_thrown_exception;
  };
};

struct oops_execution_engine_t {
  void *engine;
};

struct oops_method_t {
  void *method;
};

struct oops_class_t {
  void *clazz;
};

struct executor_options {
  uintptr_t stack_size;
};

struct oops_wrapper_t oops_invoke_method(struct oops_execution_engine_t engine,
                                         struct oops_class_t clazz,
                                         struct oops_method_t method,
                                         const struct oops_wrapper_t *args,
                                         int nargs);

#ifdef __cplusplus
}
#endif

#endif