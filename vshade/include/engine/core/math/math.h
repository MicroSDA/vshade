#ifndef ENGINE_CORE_MATH_MATH_H
#define ENGINE_CORE_MATH_MATH_H

#include <algorithm>
#include <cmath>

namespace vshade
{
namespace math
{

int binomialCoefficient(int const n, int const k)
{
    if (k > n)
        return 0;
    if (k == 0 || k == n)
        return 1;
    int c = 1;
    for (int i{0}; i < k; ++i)
    {
        c = c * (n - i) / (i + 1);
    }
    return c;
}

float calculateBezierFactor(float const value, float const min, float const max, float const lambda)
{
    float u{std::clamp((value - min) / (max - min), 0.0f, 1.0f)};
    float v{1.0f - u};
    return v * v * min + 2.0f * v * u * lambda + u * u * max;
}
} // namespace math
} // namespace vshade

#endif // ENGINE_CORE_MATH_MATH_H