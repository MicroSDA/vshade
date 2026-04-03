#ifndef ENGINE_CORE_UTILITY_UTILS_H
#define ENGINE_CORE_UTILITY_UTILS_H
#include <array>

namespace vshade
{
template <typename T, std::size_t Size> class StackArray
{
public:
    using iterator       = typename std::array<T, Size>::iterator;
    using const_iterator = typename std::array<T, Size>::const_iterator;

    StackArray()  = default;
    ~StackArray() = default;
    void pushFront(T const& item)
    {
        for (std::size_t i = array_.size() - 1U; i > 0U; --i)
            array_[i] = array_[i - 1u];
        array_[0u] = item;

        incrementSize();
    }
    void pushBack(T const& item)
    {
        for (std::size_t i = 0u; i < array_.size() - 1U; ++i)
            array_[i] = array_[i + 1U];
        array_[array_.size() - 1U] = item;

        incrementSize();
    }
    template <typename... Args> void emplaceFront(Args&&... args)
    {
        for (std::size_t i = array_.size() - 1U; i > 0U; --i)
            array_[i] = array_[i - 1U];
        array_[0U] = T(std::forward<Args>(args)...);

        incrementSize();
    }
    template <typename... Args> void emplace(Args&&... args)
    {
        array_[getSize()] = T(std::forward<Args>(args)...);
        incrementSize();
    }
    template <typename... Args> void emplaceBack(Args&&... args)
    {
        for (std::size_t i = 0U; i < array_.size() - 1U; ++i)
            array_[i] = array_[i + 1U];
        array_[array_.size() - 1U] = T(std::forward<Args>(args)...);

        incrementSize();
    }
    T& operator[](std::size_t i)
    {
        return array_[i];
    }
    T const& operator[](std::size_t i) const
    {
        return array_[i];
    }

    std::size_t getCapasity() const
    {
        return array_.size();
    }
    std::size_t getSize() const
    {
        return size_;
    }

    T const* getData() const
    {
        return array_.data();
    }
    T* getData()
    {
        return array_.data();
    }

public:
    auto begin()
    {
        return array_.begin();
    }
    auto end()
    {
        return array_.end();
    }

    auto begin() const
    {
        return array_.begin();
    }
    auto end() const
    {
        return array_.end();
    }

private:
    void incrementSize()
    {
        size_ = (size_ >= getCapasity()) ? size_ : size_ + 1U;
    }
    std::array<T, Size> array_;
    std::size_t         size_{0U};
};
} // namespace vshade

#endif // ENGINE_CORE_UTILITY_UTILS_H