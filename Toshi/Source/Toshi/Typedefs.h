#pragma once
#include <cstddef>
#include <cassert>
#include "Math/TMathInline.h"
typedef bool TBOOL;
constexpr TBOOL TFALSE = false;
constexpr TBOOL TTRUE = true;

constexpr nullptr_t TNULL = nullptr;


#define TASSERT(expression) (void)(                                                       \
            (!!(expression)) ||                                                              \
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
        )