#ifndef OOPS_CORE_EXECUTOR_H
#define OOPS_CORE_EXECUTOR_H

#include "../globals/types.h"
#include "../memory/ostack.h"
#include "../methods/method.h"

#include "../native/native_types.h"

namespace oops {
    namespace core {

        class executor {
            private:
            memory::stack vm_stack;

            public:
            bool initialize(const executor_options& options);

            oops_wrapper_t invoke(methods::method* method, const oops_wrapper_t* args, int nargs);

            void destroy();
        };
    }
}

#endif /* CORE_EXECUTOR */
