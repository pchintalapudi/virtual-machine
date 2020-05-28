#include <iostream>
#include "memory/allocator.h"

void say_hello(){
    std::cout << __cplusplus << " Hello, from oops!\n";
}

int main(int, char**) {
    say_hello();
}