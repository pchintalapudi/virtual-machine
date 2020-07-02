#ifndef UTILS_PUNS
#define UTILS_PUNS

#include <cstring>
#include <type_traits>

namespace oops {
    namespace utils {
        template<typename out_t>
        inline std::enable_if_t<std::is_trivially_copy_constructible_v<out_t> and std::is_trivially_destructible_v<out_t>, out_t> pun_read(const void* from) {
            out_t out;
            std::memcpy(&out, from, sizeof(out));
            return out;
        }

        template<typename inp_t>
        inline std::enable_if_t<std::is_trivially_copy_constructible_v<inp_t> and std::is_trivially_destructible_v<inp_t>, void> pun_write(void* to, const inp_t inp) {
            std::memcpy(to, &inp, sizeof(inp));
        }
    }
}
#endif /* UTILS_PUNS */
