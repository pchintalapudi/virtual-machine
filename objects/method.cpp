#include "method.h"

#include "class.h"

using namespace oops::objects;

oops::objects::clazz method::enclosing_class() const {
    return clazz(utils::pun_read<char*>(this->real));
}

std::uint16_t method::bytecode_length() const {
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char*));
}

std::uint16_t method::stack_frame_size() const {
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char*) + sizeof(std::uint16_t));
}

oops::objects::field::type method::get_return_type() const {
    return static_cast<field::type>(utils::pun_read<std::uint8_t>(this->real + sizeof(char*) + sizeof(std::uint16_t) * 2) & 0b111);
}

oops::objects::method::type method::get_type() const {
    return static_cast<method::type>(utils::pun_read<std::uint8_t>(this->real + sizeof(char*) + sizeof(std::uint16_t) * 2) >> 3 & 0b11);
}

std::uint16_t method::arg_count() const {
    return utils::pun_read<std::uint16_t>(this->real + sizeof(char*) + sizeof(std::uint16_t) * 3);
}

std::uint64_t method::arg_offset_pack(std::uint16_t arg_index) const {
    return utils::pun_read<std::uint64_t>(this->real + sizeof(char*) + sizeof(std::uint16_t) * 4 + static_cast<std::uint32_t>(arg_index) * sizeof(std::uint64_t));
}

char* method::bytecode_begin() const {
    constexpr std::uint32_t types_per_instr = sizeof(std::uint64_t) * CHAR_BIT / 3;
    std::uint32_t arg_count = this->arg_count();
    auto skip_types = (arg_count + types_per_instr - 1) / types_per_instr;
    return this->real + sizeof(char*) + sizeof(std::uint16_t) * 4 + skip_types * sizeof(std::uint64_t);
}