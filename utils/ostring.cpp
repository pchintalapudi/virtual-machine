#include "ostring.h"

#include "puns.h"

using namespace oops::utils;

char ostring::operator[](std::uint32_t idx) const
{
    return this->real[idx];
}

std::uint32_t ostring::length() const
{
    return utils::pun_read<std::uint32_t>(this->real - sizeof(std::uint32_t));
}

bool ostring::operator==(const ostring &other) const {
    auto length = this->length();
    return length == other.length() and std::memcmp(this->real, other.real, length) == 0;
}
bool ostring::operator!=(const ostring &other) const {
    return !(*this == other);
}
bool ostring::operator<(const ostring &other) const {
    auto length = this->length();
    auto other_length = other.length();
    auto cmp = std::memcmp(this->real, other.real, std::min(length, other_length));
    return cmp < 0 or (cmp == 0 and length < other_length);
}
bool ostring::operator<=(const ostring &other) const {
    auto length = this->length();
    auto other_length = other.length();
    auto cmp = std::memcmp(this->real, other.real, std::min(length, other_length));
    return cmp < 0 or (length <= other_length and cmp == 0);
}
bool ostring::operator>(const ostring &other) const {
    return other <= *this;
}
bool ostring::operator>=(const ostring &other) const {
    return other < *this;
}

int ostring::compare_to(const ostring &other) const {
    return *this < other ? -1 : *this == other ? 0 : 1;
}