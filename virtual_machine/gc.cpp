#include "vm.h"
using namespace oops::virtual_machine;

bool virtual_machine::gc(bool force_old)
{
    if (force_old)
    {
        //Mark static references
        //Mark stack
        //Walk heaps and compact young generation
    }
    else
    {
        //Walk static references
        //Walk cross-generational references
        //Walk stack
        //Walk survivors
    }
}