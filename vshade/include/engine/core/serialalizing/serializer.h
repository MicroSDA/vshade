#ifndef ENGINE_CORE_SERIALIZING_SERIALIZER_H
#define ENGINE_CORE_SERIALIZING_SERIALIZER_H
#include <fstream>
#include <type_traits>
#include <limits>

namespace vshade
{
namespace serializer
{
struct Serializer
{
    template <typename T> static void serialize(std::ostream& stream, T const& object)
    {
        if constexpr (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>)
        {
            stream.write(reinterpret_cast<char const*>(&object), sizeof(T));
        }
        else
        {
            // Assertion to ensure the type is serializable
            static_assert(
                false, "You are trying to serialize a type that is not trivial or raw pointer and does not have a special Serialize specialization!");
        }
    }
    template <typename T> static void serialize(std::ostream& stream, T const& object, std::size_t count)
    {
        if constexpr (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>)
        {
            stream.write(reinterpret_cast<char const*>(&object), sizeof(T) * count);
        }
        else
        {
            // Assertion to ensure the type is serializable
            static_assert(
                false, "You are trying to serialize a type that is not trivial or raw pointer and does not have a special Serialize specialization!");
        }
    }
    template <typename T> static void deserialize(std::istream& stream, T& object)
    {
        if constexpr (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>)
        {
            stream.read(reinterpret_cast<char*>(&object), sizeof(T));
        }
        else
        {
            // Assertion to ensure the type is deserializable
            static_assert(
                false,
                "You are trying to deserialize a type that is not trivial or raw pointer and does not have a special Deserialize specialization!");
        }
    }
    template <typename T> static void deserialize(std::istream& stream, T& object, std::size_t count)
    {
        if constexpr (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>)
        {
            stream.read(reinterpret_cast<char*>(&object), sizeof(T) * count);
        }
        else
        {
            // Assertion to ensure the type is deserializable
            static_assert(
                false,
                "You are trying to deserialize a type that is not trivial or raw pointer and does not have a special Deserialize specialization!");
        }
    }
};
} // namespace serializer
} // namespace vshade

namespace vshade
{
namespace serializer
{
template <> inline void Serializer::serialize(std::ostream& stream, std::string const& string)
{
    std::uint32_t size = static_cast<std::uint32_t>(string.size());
    if (size == std::numeric_limits<std::uint32_t>::max())
        throw std::out_of_range("Incorrect string size !");

    // Write the string data including the null terminator
    if (size)
        stream.write(string.data(), sizeof(char) * (string.size() + 1));
    else
        stream.write("\0", sizeof(char) * 1);
}

template <> inline void Serializer::deserialize(std::istream& stream, std::string& string)
{
    std::streampos const begin  = stream.tellg();
    char                 symbol = ' ';
    // Locate the null terminator
    do
    {
        stream.read(&symbol, sizeof(char));
    } while (symbol != '\0' && stream.good());

    // Calculate the size of the string
    std::streampos const end  = stream.tellg();
    std::uint32_t const  size = std::uint32_t((end - begin) - 1);

    if (size + 1 == std::numeric_limits<std::uint32_t>::max() || size < 0)
        throw std::out_of_range("Incorrect string size !");

    if (size)
    {
        string.resize(size);
        stream.seekg(begin);
        stream.read(string.data(), sizeof(char) * (string.size()));
        stream.seekg(end);
    }
}
} // namespace serializer
} // namespace vshade

#endif // ENGINE_CORE_SERIALIZING_SERIALIZER_H