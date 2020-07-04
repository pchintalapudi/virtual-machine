#ifndef UTILS_OITERATOR
#define UTILS_OITERATOR

#include <iterator>
#include <type_traits>
#include "puns.h"

namespace oops {
    namespace utils {
        template<typename primitive>
        class const_aliased_iterator {
            static_assert(std::is_arithmetic_v<primitive>);

            private:
            char* real;
            public:
            const_aliased_iterator(char* real) : real(real) {}

            typedef std::ptrdiff_t difference_type;
            typedef primitive value_type;
            typedef const primitive* pointer;
            typedef const primitive& reference;
            typedef std::random_access_iterator_tag iterator_category;

            const_aliased_iterator& operator++() {
                this->real += sizeof(primitive);
                return *this;
            }

            primitive operator*() const {
                return pun_read<primitive>(this->real);
            }

            bool operator==(const const_aliased_iterator& other) const {
                return other.real == this->real;
            }

            bool operator!=(const const_aliased_iterator& other) const {
                return other.real != this->real;
            }

            const_aliased_iterator operator++(int) {
                const_aliased_iterator ret(this->real);
                this->real += sizeof(primitive);
                return ret;
            }

            const_aliased_iterator& operator--() {
                this->real -= sizeof(primitive);
                return *this;
            }

            const_aliased_iterator operator--(int) {
                const_aliased_iterator ret(this->real);
                this->real -= sizeof(primitive);
                return ret;
            }

            const_aliased_iterator& operator+=(difference_type n) {
                this->real += n * sizeof(primitive);
                return *this;
            }

            const_aliased_iterator operator+(difference_type n) {
                return const_aliased_iterator(this->real + n * sizeof(primitive));
            }

            friend const_aliased_iterator operator+(difference_type n, const_aliased_iterator a) {
                return const_aliased_iterator(a.real + n * sizeof(primitive));
            }

            const_aliased_iterator& operator-=(difference_type n) {
                this->real -= n * sizeof(primitive);
                return *this;
            }

            const_aliased_iterator operator-(difference_type n) {
                return const_aliased_iterator(this->real - n * sizeof(primitive));
            }

            difference_type operator-(const const_aliased_iterator& other) {
                return (this->real - other.real) / sizeof(primitive);
            }

            primitive operator[](difference_type n) {
                return pun_read<primitive>(this->real + n * sizeof(primitive));
            }

            bool operator<(const const_aliased_iterator& other) {
                return this->real < other.real;
            }

            bool operator>(const const_aliased_iterator& other) {
                return this->real > other.real;
            }

            bool operator<=(const const_aliased_iterator& other) {
                return this->real <= other.real;
            }

            bool operator>=(const const_aliased_iterator& other) {
                return this->real >= other.real;
            }
        };
    }
}
#endif /* UTILS_OITERATOR */
