#include "ostack.h"

using namespace oops::memory;

oops::objects::method frame::get_method() const {
    return objects::method(utils::pun_read<char*>(this->real));
}

std::uint16_t frame::return_address() const {
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char*));
}
std::uint16_t frame::return_offset() const {
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char*) + sizeof(std::uint16_t));
}

std::uint16_t frame::size() const {
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char*) + sizeof(std::uint16_t) * 2);
}

char* frame::data_start() const {
    return this->real + sizeof(char*) + sizeof(std::uint16_t*) * 4;
}