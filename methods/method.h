#ifndef OOPS_METHODS_METHOD_H
#define OOPS_METHODS_METHOD_H

#include "../instructions/instructions.h"
#include "../memory/byteblock.h"


namespace oops {
    namespace methods {

        class method {
            private:
            memory::byteblock<false> location;
            public:
            method(void* ptr);
            instructions::instruction read_instruction(instr_idx_t offset);
        };
    }
}

#endif