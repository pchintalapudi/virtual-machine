#ifndef PLATFORM_SPECIFIC_FILES
#define PLATFORM_SPECIFIC_FILES
#include <optional>

#include "../utils/utils.h"

namespace oops
{
    namespace platform
    {
        struct file_mapping {
            char* mmapped_file;
            void* _file_map_handle;
            void* _file_handle;
        };

        std::optional<file_mapping> open_file_mapping(oops::utils::ostring name);

        void close_file_mapping(file_mapping fm);
    } // namespace platform
} // namespace oops
#endif /* PLATFORM_SPECIFIC_FILES */
