
#include "random.h"

static unsigned random_uint32_range(unsigned a, unsigned b)
{
    return (random_uint32() % (b - a)) + a;
}

