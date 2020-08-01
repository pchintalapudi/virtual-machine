#include "files.h"

using namespace oops::platform;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "windows.h"

std::optional<file_mapping> oops::platform::open_file_mapping(oops::utils::ostring name)
{
    std::string lpcstr;
    lpcstr.reserve(name.length() + sizeof(".coops"));
    for (char c : name)
    {
        if (c == '.')
        {
            lpcstr += '/';
        }
        else
        {
            lpcstr += c;
        }
    }
    lpcstr += ".coops";
    void *file_handle = CreateFileA(lpcstr.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        return {};
    }
    void *file_map_handle = CreateFileMappingA(file_handle, NULL, PAGE_READONLY, 0, 0, lpcstr.c_str());
    if (file_map_handle == NULL)
    {
        CloseHandle(file_handle);
        return {};
    }
    void *mmap_handle = MapViewOfFile(file_map_handle, FILE_MAP_READ, 0, 0, 0);
    if (mmap_handle == NULL)
    {
        CloseHandle(file_map_handle);
        CloseHandle(file_handle);
        return {};
    }
    return {{static_cast<char *>(mmap_handle), file_map_handle, file_handle}};
}

void oops::platform::close_file_mapping(file_mapping fm) {
    UnmapViewOfFile(fm.mmapped_file);
    CloseHandle(fm._file_map_handle);
    CloseHandle(fm._file_handle);
}

#endif