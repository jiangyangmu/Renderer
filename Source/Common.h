#pragma once

#include <cassert>

#ifdef NDEBUG

#define assert(e) if (!(e)) { MessageBox(NULL, TEXT(#e), TEXT("Exception"), MB_OK); }

#endif

#define ASSERT(e) assert((e))
#define ENSURE_NOT_NULL(e) assert((e) != NULL)
#define ENSURE_TRUE(e) assert((e))
#define ENSURE_GDIPLUS_OK(e) assert((e) == Gdiplus::Ok)
