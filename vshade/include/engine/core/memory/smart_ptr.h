#ifndef ENGINE_CORE_MEMORY_SMART_PTR_H
#define ENGINE_CORE_MEMORY_SMART_PTR_H

#include <memory>
#include <engine/core/memory/allocation.h>

namespace vshade
{
    template <typename T>
    class Shared
    {
    public:
        explicit Shared() = default;
        virtual ~Shared() = default;

        template <typename... Args>
        static Shared<T> create(Args &&...args)
        {
            return Shared{VS_NEW T{std::forward<Args>(args)...}};
        }

        // @brief Copy constructor for related types (e.g., base or derived classes).
		// @tparam U Type that is related to T.
		template<typename U, std::enable_if_t<std::is_base_of_v<U, T> || std::is_base_of_v<T, U>, bool> = true>
		Shared(const Shared<U>&other)
		{
			instance_ptr_ = std::static_pointer_cast<T>(other.instance_ptr_);
		}
		// @brief Copy assignment operator for related types.
		// @tparam U Type that is related to T.
		template<typename U, std::enable_if_t<std::is_base_of_v<U, T> || std::is_base_of_v<T, U>, bool> = true>
		Shared & operator=(const Shared<U>&other)
		{
			if (instance_ptr_ != other.instance_ptr_)
			{
				instance_ptr_ = std::static_pointer_cast<T>(other.instance_ptr_);
			}
			return *this;
		}

    private:
        explicit Shared(T *ptr) { instance_ptr_.reset(ptr); };
        std::shared_ptr<T> instance_ptr_;

         template<class U>
		friend class Shared;
    };
}

#endif // ENGINE_CORE_MEMORY_SMART_PTR_H
