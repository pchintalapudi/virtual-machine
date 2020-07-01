#include "class_manager.h"

bool oops::classes::class_manager:: instanceof (oops::objects::clazz cls, oops::objects::clazz type)
{
    auto class_index = class_indexer[cls];
    auto begin = relations.begin() + class_index + 1;
    auto end = begin + relations[class_index];
    char *type_pointer = type.unwrap();
    PUN(std::uintptr_t, search, &type_pointer);
    return std::binary_search(begin, end, search);
}