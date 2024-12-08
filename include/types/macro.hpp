#pragma once

#include <types/fast_fixed.hpp>
#include <types/fixed.hpp>

#define FLOAT float
#define DOUBLE double
#define FIXED(n, k) Fixed<n, k>
#define FAST_FIXED(n, k) FastFixed<n, k>
