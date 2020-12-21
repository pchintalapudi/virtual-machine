#include "instanceof_table.h"

using namespace oops::classloading;

bool instanceof_table::is_superclass(classes::clazz super, classes::clazz sub) {
    auto index_and_length = this->class_indeces.find(super.get_raw());
    if (index_and_length == this->class_indeces.end()) {
        return false;
    }
    auto begin = this->classes.begin() + index_and_length->second.first;
    auto end = begin + index_and_length->second.second;
    auto present = std::lower_bound(begin, end, sub.get_raw());
    return present != end && *present == sub.get_raw();
}
