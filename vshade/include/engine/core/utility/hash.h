
#ifndef ENGINE_CORE_UTILITY_HASH_H
#define ENGINE_CORE_UTILITY_HASH_H

#include <glm/glm/gtx/hash.hpp>
#include <iostream>

namespace vshade
{
namespace utils
{
/**
 * @brief Hash function for std::pair<T, U>, compatible with unordered maps.
 */
struct PairHash final
{
    template <typename T, typename U> std::size_t operator()(std::pair<T, U> const& pair) const
    {
        std::size_t h1 = std::hash<T>{}(pair.first);
        std::size_t h2 = std::hash<U>{}(pair.second);
        return h1 + 0x9e3779b9 + (h2 << 6U) + (h2 >> 2U); ///< Similar to boost::hash_combine
    }
};

/**
 * @brief Utility function to combine multiple hash values into one.
 * @tparam Args Types of values to hash.
 * @param args Values to hash.
 * @return Combined hash value.
 */

template <typename... Args> std::size_t hashCombine(Args&&... args)
{
    // std::size_t seed         = 0U;
    // auto        hash_combine = [&seed](auto&& value)
    // {
    //     std::size_t h = std::hash<std::decay_t<decltype(value)>>{}(value);
    //     seed ^= h + 0x9e3779b9 + (seed << 6U) + (seed >> 2U);
    // };

    // (hash_combine(std::forward<Args>(args)), ...);
    // return seed;

    return 0U;
}

template <typename T> void hash_combine(std::size_t& seed, T const& hash)
{
    std::hash<T> hasher;
    glm::detail::hash_combine(seed, hasher(hash));
}

template <typename T>
constexpr std::size_t to_hash(T&& value) {
    if constexpr (std::is_same_v<std::decay_t<T>, std::size_t>)
        return value;
    else
        return std::hash<std::decay_t<T>>{}(std::forward<T>(value));
}
template <typename... Args>
std::size_t hash(Args&&... args) {
    return (to_hash(std::forward<Args>(args)) ^ ...);
}

} // namespace utils

} // namespace vshade

#endif // ENGINE_CORE_UTILITY_HASH_H