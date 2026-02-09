#include "vec2.h"

float vec2_length(vec2_t vec)
{
    return sqrt((vec.v0 * vec.v0) + (vec.v1 * vec.v1));
}