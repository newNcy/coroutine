#pragma once
#include <stdio.h>

typedef void * any_t;
#define nullptr NULL

typedef int (*any_compare_t) (any_t lhs, any_t rhs);

