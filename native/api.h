#ifndef NATIVE_API
#define NATIVE_API
#ifdef __cplusplus
extern "C"
{
#endif
#include "stdint.h"
    struct oops_vm
    {
        void *vm;
    };
    struct oops_result
    {
        union {
            int32_t int_value;
            int64_t long_value;
            float float_value;
            double double_value;
            void *object_value;
        };
        unsigned type;
        unsigned status;
    };

    #ifndef __cplusplus
    typedef (struct oops_result) (*native_function)(struct oops_vm vm);
    #else
    typedef oops_result (*native_function)(oops_vm vm);
    #endif

#ifdef __cplusplus
}
#endif
#endif /* NATIVE_API */
