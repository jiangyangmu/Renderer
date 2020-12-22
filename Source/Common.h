#pragma once

#include <cassert>

#define ASSERT(e) assert((e))
#define ENSURE_NOT_NULL(e) assert((e) != NULL)
#define ENSURE_TRUE(e) assert((e))
#define ENSURE_GDIPLUS_OK(e) assert((e) == Gdiplus::Ok)
