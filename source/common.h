#pragma once

#ifdef __INTELLISENSE__
#define __STDC__
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

#define cast reinterpret_cast

typedef char *LPSTR;
typedef const char *LPCSTR;
typedef uint8_t *ptr;

#define __STATIC_ASSERT(expr) typedef char __static_assert[expr ? 1 : -1];

#define STRUCT_TYPE(x) x
#define STRUCT_SIZE(struc, size) __STATIC_ASSERT(sizeof(STRUCT_TYPE(struc)) == size)

typedef std::basic_string<uint16_t, std::char_traits<uint16_t>, std::allocator<uint16_t> > wide_string;

#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wuninitialized"

