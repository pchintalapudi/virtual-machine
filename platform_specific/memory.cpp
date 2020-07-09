#include "memory.h"

using namespace oops::platform;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "windows.h"

oops::platform::platform_info oops::platform::get_platform_info()
{
    SYSTEM_INFO sys_info;
    GetNativeSystemInfo(&sys_info);
    return {.page_size = sys_info.dwPageSize, .allocation_granularity = sys_info.dwAllocationGranularity, .processor_count = sys_info.dwNumberOfProcessors};
}

std::optional<char *> oops::platform::reserve(std::size_t amount)
{
    if (void *value = VirtualAlloc(nullptr, amount, MEM_RESERVE, PAGE_READWRITE); value)
        return static_cast<char*>(value);
    return {};
}

bool oops::platform::commit(char* memory, std::size_t amount) {
    return VirtualAlloc(memory, amount, MEM_COMMIT, PAGE_READWRITE) != nullptr;
}

void oops::platform::decommit(char* memory, std::size_t amount) {
    VirtualFree(memory, amount, MEM_DECOMMIT);
}

void oops::platform::dereserve(char* memory) {
    VirtualFree(memory, 0, MEM_RELEASE);
}
#endif