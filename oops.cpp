#include <iostream>
#include <map>
// #include "memory/allocator.h"

void say_hello(){
    std::cout << sizeof(std::map<char*, unsigned long long>) << " Hello, from oops!\n";
}

int main(int, char**) {
    say_hello();
}