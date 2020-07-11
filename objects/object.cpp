#include "object.h"
#include "../memory/memutils.h"

using namespace oops::objects;

clazz base_object::get_clazz() const {
    return utils::pun_reinterpret<char*>(utils::pun_reinterpret<std::uintptr_t>(utils::pun_read<char*>(this->real)) >> 3 << 3);
}

std::uint8_t base_object::metadata() const {
    return utils::pun_reinterpret<std::uintptr_t>(utils::pun_read<char*>(this->real)) & 0b111;
}

std::uint64_t base_object::malloc_size() const {
    return memory::size64to32(utils::pun_read<std::uint32_t>(this->real - sizeof(std::uint32_t)));
}

std::uint32_t base_object::tail_data() const {
    return utils::pun_read<std::uint32_t>(this->real + this->malloc_size());
}

std::uint64_t array::size(field::type atype, std::uint32_t length) {
    std::uint64_t out = length;
    switch(atype) {
        default:
        return 0;
        case field::type::CHAR:
        out *= sizeof(std::int8_t);
        break;
        case field::type::SHORT:
        out *= sizeof(std::int16_t);
        break;
        case field::type::INT:
        out *= sizeof(std::int32_t);
        break;
        case field::type::FLOAT:
        out *= sizeof(float);
        break;
        case field::type::LONG:
        out *= sizeof(std::int64_t);
        break;
        case field::type::DOUBLE:
        out *= sizeof(double);
        break;
        case field::type::OBJECT:
        out *= sizeof(char*);
        break;
    }
    out += sizeof(char*) + sizeof(std::uint32_t) * 2;
    return out;
}