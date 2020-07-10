#include "instanceof.h"

#include <algorithm>
#include "../utils/utils.h"

using namespace oops::interfaze;

bool instanceof::operator()(objects::clazz src, objects::clazz test) const {
    if (auto it = this->class_index.find(src.unwrap()); it != this->class_index.end()) {
        auto impl_it = this->implemented.begin() + it->second;
        return std::binary_search(impl_it + 1, impl_it + *impl_it, utils::pun_reinterpret<std::uintptr_t>(test.unwrap()));
    }
}

void instanceof::insert_new_class(objects::clazz cls, std::size_t length, void *classes) {
    this->class_index[cls.unwrap()] = this->implemented.size();
    this->implemented.push_back(length);
    char* cptr = static_cast<char*>(classes);
    for (std::size_t i = 0; i < length; i++) {
        this->implemented.push_back(utils::pun_read<std::uintptr_t>(cptr));
        cptr += sizeof(char*);
    }
}