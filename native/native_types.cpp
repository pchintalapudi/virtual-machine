#include "native_types.h"
#include "../core/executor.h"

struct oops_wrapper_t oops_invoke_method(struct oops_execution_engine_t engine, struct oops_method_t method, const struct oops_wrapper_t* args, int nargs) {
    oops::core::executor* executor = static_cast<oops::core::executor*>(engine.engine);
    oops::methods::method real_method = oops::methods::method(method.method);
    return executor->invoke(real_method, args, nargs);
}
