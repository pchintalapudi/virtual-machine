#include "ostack.h"

using namespace oops::memory;

oops::objects::method frame::get_method() {
    return objects::method(utils::pun_read<char*>(this->real));
}

std::uint16_t frame::return_address() {
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char*));
}
std::uint16_t frame::return_offset() {
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char*) + sizeof(std::uint16_t));
}