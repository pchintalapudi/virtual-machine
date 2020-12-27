#if !defined(OOPS_COMPILE_FOR_WINDOWS) && !defined(OOPS_COMPILE_FOR_MAC) && \
    !defined(OOPS_COMPILE_FOR_LINUX)
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
#define OOPS_COMPILE_FOR_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#define OOPS_COMPILE_FOR_MAC
#define OOPS_COMPILE_FOR_UNIX
#elif defined(__linux__)
#define OOPS_COMPILE_FOR_LINUX
#define OOPS_COMPILE_FOR_UNIX
#endif
#endif

#if !!defined(OOPS_COMPILE_FOR_WINDOWS) + !!defined(OOPS_COMPILE_FOR_MAC) + !!defined(OOPS_COMPILE_FOR_LINUX) > 1
#error MULTIPLE OPERATING SYSTEMS DEFINED!!!
#endif

#if !!defined(OOPS_COMPILE_FOR_WINDOWS) + !!defined(OOPS_COMPILE_FOR_MAC) + !!defined(OOPS_COMPILE_FOR_LINUX) < 1
#error UNSUPPORTED OPERATING SYSTEM DETECTED!!!
#endif