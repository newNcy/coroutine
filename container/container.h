#pragma once
#include <stdio.h>

typedef unsigned long long any_t;
#define nullptr NULL

typedef int (*any_compare_t) (any_t lhs, any_t rhs);

static inline int less(any_t lhs, any_t rhs) 
{
    return lhs < rhs;
}

static inline int equals(any_t lhs, any_t rhs) 
{
    return lhs == rhs;
}
