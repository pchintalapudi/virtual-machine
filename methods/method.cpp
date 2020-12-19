#include "method.h"

using namespace oops::methods;

method::method(void* ptr) {
    this->location.initialize(ptr);
}