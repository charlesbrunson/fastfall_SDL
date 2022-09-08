#pragma once

#include <memory>
#include <concepts>

namespace ff {

// uses type erasure to allow copying
// fatter than regular unique_ptr as result

template<class Base>
struct copyable_unique_ptr {
	copyable_unique_ptr() = default;

	template<std::copy_constructible Type>
		requires (std::derived_from<Type, Base> && !std::same_as<Type, Base>)
	copyable_unique_ptr(Type* in_ptr)
	{
		clone = make_copy_fn<Type>();
		ptr = in_ptr;
	}

	template<std::copy_constructible Type>
        requires (std::derived_from<Type, Base> && !std::same_as<Type, Base>)
	copyable_unique_ptr(std::unique_ptr<Type>&& in_ptr)
	{
		clone = make_copy_fn<Type>();
		ptr = std::move(in_ptr);
	}
	
	copyable_unique_ptr(const copyable_unique_ptr<Base>& other)
	{
		clone = other.clone;
		ptr = clone(other.ptr);
	}

	copyable_unique_ptr(copyable_unique_ptr<Base>&& other)
	{
		clone = other.clone;
		ptr = std::move(other.ptr);
	}

	copyable_unique_ptr<Base>& operator=(const copyable_unique_ptr<Base>& other)
	{
		clone = other.clone;
		ptr = clone(other.ptr);
		return *this;
	}

	copyable_unique_ptr<Base>& operator=(copyable_unique_ptr<Base>&& other) {
		clone = other.clone;
		ptr = std::move(other.ptr);
		return *this;
	}

	copyable_unique_ptr<Base>& operator=(nullptr_t) {
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
		return ptr;
	}

private:
	using clone_fn = std::unique_ptr<Base>(const std::unique_ptr<Base>& ptr);

	std::unique_ptr<Base> ptr = nullptr;
	clone_fn* clone = nullptr;

	template<std::copy_constructible Type>
        requires (std::derived_from<Type, Base> && !std::same_as<Type, Base>)
	static auto make_copy_fn()
	{
		return [](const std::unique_ptr<Base>& ptr) -> std::unique_ptr<Base> {
			const Type* under_ptr = static_cast<const Type*>(ptr.get());
			return std::make_unique<Type>(*under_ptr);
		};
	}
};

template<std::copy_constructible Type, class... Args>
copyable_unique_ptr<Type> make_copyable_unique(Args&&... args)
{
	return copyable_unique_ptr<Type>(new Type(std::forward<Args>(args)...));
}

}