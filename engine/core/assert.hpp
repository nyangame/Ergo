#pragma once
#include "log.hpp"
#include <cstdlib>

#define ERGO_ASSERT(cond, msg) \
    do { if (!(cond)) { \
        ergo::log::fatal("ASSERT", "%s:%d: %s", __FILE__, __LINE__, msg); \
        std::abort(); \
    } } while(0)

#ifdef NDEBUG
#define ERGO_DEBUG_ASSERT(cond, msg) ((void)0)
#else
#define ERGO_DEBUG_ASSERT(cond, msg) ERGO_ASSERT(cond, msg)
#endif
