#pragma once
//https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/generator.hpp
namespace fast_io
{

template<typename T>
class generator;

namespace details
{
template<typename T>
class generator_promise
{
public:
	using element_type = T;
	using value_type = std::remove_cvref_t<element_type>;
	using reference = std::remove_reference_t<T>&;
	using const_reference = std::remove_reference_t<T> const&;
	using pointer = value_type*;
	using const_pointer = value_type const*;
	pointer ptr{};
#ifdef __cpp_exceptions
	std::exception_ptr ex_ptr;
#endif
	generator<T> get_return_object() noexcept;

	constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
	constexpr std::suspend_always final_suspend() const noexcept { return {}; }
	template<typename U>
	requires std::same_as<std::remove_reference_t<U>,std::remove_reference_t<T>>
	constexpr std::suspend_always yield_value(U&& value) noexcept
	{
		ptr = std::addressof(value);
		return {};
	}
	constexpr void unhandled_exception() noexcept
	{
#ifdef __cpp_exceptions
		ex_ptr = std::current_exception();
#else
		std::terminate();
#endif
	}
	constexpr void return_void() noexcept
	{
	}
	template<typename U>
	std::suspend_never await_transform(U&& value) = delete;

	void rethrow_if_exception()
	{
#ifdef __cpp_exceptions
		if (ex_ptr)
			std::rethrow_exception(ex_ptr);
#endif
	}
};
class generator_sentinel{};

template<typename T>
class generator_iterator
{
public:
	using coroutine_handle_type = std::coroutine_handle<generator_promise<T>>;
	using iterator_category = std::output_iterator_tag;
	coroutine_handle_type handle{};

	inline constexpr auto operator->()
	{
		handle.resume();
#ifdef __cpp_exceptions
		if (handle.done())
			handle.promise().rethrow_if_exception();
#endif
		return handle.promise().ptr;
	}

	inline constexpr auto& operator*()
	{
		handle.resume();
#ifdef __cpp_exceptions
		if (handle.done())
			handle.promise().rethrow_if_exception();
#endif
		return *(handle.promise().ptr);
	}

};

template<typename T>
inline constexpr bool operator==(generator_sentinel, generator_iterator<T> const& b) noexcept
{
	return b.handle.done();
}

template<typename T>
inline constexpr bool operator==(generator_iterator<T> const& b,generator_sentinel) noexcept
{
	return b.handle.done();
}
template<typename T>
inline constexpr bool operator!=(generator_sentinel s, generator_iterator<T> const& b) noexcept
{
	return !(s==b);
}

template<typename T>
inline constexpr bool operator!=(generator_iterator<T> const& b,generator_sentinel s) noexcept
{
	return !(s==b);
}

template<typename T>
inline constexpr generator_iterator<T>& operator++(generator_iterator<T>& b)
{
	return b;
}
template<typename T>
inline constexpr void operator++(generator_iterator<T>& b,int)
{
}

}

template<typename T>
class [[nodiscard]] generator
{
public:
	using promise_type = details::generator_promise<T>;
	std::coroutine_handle<promise_type> handle;
	constexpr generator(nullptr_t)=delete;
	constexpr generator(std::coroutine_handle<promise_type> v):handle(v){}
	constexpr generator(generator const&) noexcept=delete;
	constexpr generator& operator=(generator const&) noexcept=delete;
	constexpr ~generator()
	{
		handle.destroy();
	}
};
template<typename T>
inline constexpr details::generator_iterator<T> begin(generator<T>& gen)
{
	return {gen.handle};
}
template<typename T>
inline constexpr details::generator_sentinel end(generator<T>& gen)
{
	return {};
}
template<typename T>
inline constexpr details::generator_iterator<T> cbegin(generator<T> const& gen)
{
	return {gen.handle};
}
template<typename T>
inline constexpr details::generator_sentinel cend(generator<T> const& gen)
{
	return {};
}
template<typename T>
inline constexpr details::generator_iterator<T> begin(generator<T> const& gen)
{
	return {gen.handle};
}
template<typename T>
inline constexpr details::generator_sentinel end(generator<T> const& gen)
{
	return {};
}

namespace details
{
template<typename T>
inline generator<T> generator_promise<T>::get_return_object() noexcept
{
	using coroutine_handle = std::coroutine_handle<generator_promise<T>>;
	return { coroutine_handle::from_promise(*this) };
}
}
}