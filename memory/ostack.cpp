#include "ostack.h"

using namespace oops::memory;

stack::frame &stack::current_frame() { return this->current; }