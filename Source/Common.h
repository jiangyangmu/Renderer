#pragma once

// --------------------------------------------------------------------------
// Common type aliases, constants
// --------------------------------------------------------------------------

#include <cinttypes>

typedef unsigned char Byte;
typedef int64_t Integer;

constexpr auto PI = 3.141592653f;

#include <memory>

template <typename T>
using Ptr = std::unique_ptr<T>;
template <typename T>
using Ref = std::shared_ptr<T>; // TODO: implement a non thread-safe one

// --------------------------------------------------------------------------
// Macros for debugging
// --------------------------------------------------------------------------

#include <cassert>

#define _ALERT_IF_FALSE(e) if (!(e)) { MessageBox(NULL, TEXT(#e), TEXT("Exception"), MB_OK); ExitProcess(0); }

#define ENSURE_NOT_NULL(e) _ALERT_IF_FALSE((e) != NULL)
#define ENSURE_TRUE(e) _ALERT_IF_FALSE((e))
#define ENSURE_GDIPLUS_OK(e) _ALERT_IF_FALSE((e) == Gdiplus::Ok)

//#define _RELEASE_ASSERT
#ifdef _RELEASE_ASSERT

#ifdef NDEBUG
#define ASSERT(e) _ALERT_IF_FALSE(e)
#else
#define ASSERT(e) assert(e)
#endif

#else

#define ASSERT(e) assert(e)

#endif
