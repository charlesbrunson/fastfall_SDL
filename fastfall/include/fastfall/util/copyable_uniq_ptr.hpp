#pragma once

#include <memory>
#include <concepts>

#include <iostream>

namespace ff {

// uses type erasure to allow copying
// fatter than regular unique_ptr as result

template<class Base>
struct copyable_unique_ptr {

    template<class Derived>
    friend struct copyable_unique_ptr;

	copyable_unique_ptr() = default;

	template<std::copy_constructible Type>
	explicit copyable_unique_ptr(Type* in_ptr)
	{
		clone = make_copy_fn<Type>();
		ptr = std::unique_ptr<Type>{ in_ptr };
	}

	copyable_unique_ptr(const copyable_unique_ptr<Base>& other)
	{
        if (this != &other) {
            clone = other.clone;

            if (other.ptr)
                ptr = clone(other.ptr);
        }
	}

	copyable_unique_ptr(copyable_unique_ptr<Base>&& other) noexcept
	{
		clone = other.clone;
        ptr = std::move(other.ptr);
        other.clone = nullptr;
	}

    constexpr copyable_unique_ptr(std::nullopt_t) noexcept
    {
        clone = nullptr;
        ptr = nullptr;
    }

	copyable_unique_ptr<Base>& operator=(const copyable_unique_ptr<Base>& other)
	{
        if (this == &other)
            return *this;

		clone = other.clone;

        if (other.ptr)
		    ptr = clone(other.ptr);
		return *this;
	}

	copyable_unique_ptr<Base>& operator=(copyable_unique_ptr<Base>&& other) noexcept {
		clone = other.clone;
        ptr = std::move(other.ptr);
        other.clone = nullptr;
		return *this;
	}

	copyable_unique_ptr<Base>& operator=(std::nullptr_t) {
		clone = nullptr;
		ptr = nullptr;
		return *this;
	}

	Base* operator->() {
		return ptr.get();
	}
	const Base* operator->() const {
		return ptr.get();
	}

	Base& operator*() {
		return *ptr;
	}
	const Base& operator*() const {
		return *ptr;
	}

	void reset() {
		ptr.reset();
		clone = nullptr;
	}

	Base* get() {
		return ptr.get();
	}
	const Base* get() const {
		return ptr.get();
	}

	operator bool() {
		return ptr != nullptr;
	}

private:
	using clone_fn = std::unique_ptr<Base>(const std::unique_ptr<Base>& ptr);

	std::unique_ptr<Base> ptr;
	clone_fn* clone = nullptr;

	template<std::copy_constructible Type>
        requires (std::derived_from<Type, Base> || std::same_as<Type, Base>)
	static auto make_copy_fn()
	{
		return [](const std::unique_ptr<Base>& clone_ptr) -> std::unique_ptr<Base> {
			const Type* under_ptr = static_cast<const Type*>(clone_ptr.get());
			return std::make_unique<Type>(*under_ptr);
		};
	}
};

template<class Base, std::derived_from<Base> Derived, class... Args>
copyable_unique_ptr<Base> make_copyable_unique(Args&&... args) {
    return copyable_unique_ptr<Base>{ new Derived{std::forward<Args>(args)...} };
}


}