#include "object.h"

using namespace oops::objects;

clazz base_object::get_clazz() const {
    return utils::pun_read<char*>(this->real);
}