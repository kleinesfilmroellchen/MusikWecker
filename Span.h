/*
 * Copyright (c) 2020-2021, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <array>
#include <memory>
#include <stdint.h>
#include <type_traits>
#include <vector>

// start of fixups to make Serenity's AK code work with C++17, std and Arduino
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define VERIFY(expr) __builtin_expect(!(expr), 0)
// #define HAS_CPP20 (__cplusplus >= 202002L)
#define HAS_CPP20 false

using FlatPtr = uintptr_t;
static_assert(sizeof(FlatPtr) == sizeof(void*));
// end of fixups

// Types.h

constexpr size_t align_up_to(const size_t value, const size_t alignment)
{
	return (value + (alignment - 1)) & ~(alignment - 1);
}

constexpr size_t align_down_to(const size_t value, const size_t alignment)
{
	return value & ~(alignment - 1);
}

// end of Types.h

// TypedTransfer.h

template <typename T>
class TypedTransfer {
public:
	static void move(T* destination, T* source, size_t count)
	{
		if (count == 0)
			return;

		if constexpr (std::is_trivial_v<T>) {
			__builtin_memmove(destination, source, count * sizeof(T));
			return;
		}

		for (size_t i = 0; i < count; ++i) {
			if (destination <= source)
				new (&destination[i]) T(std::move(source[i]));
			else
				new (&destination[count - i - 1]) T(std::move(source[count - i - 1]));
		}
	}

	static size_t copy(T* destination, T const* source, size_t count)
	{
		if (count == 0)
			return 0;

		if constexpr (std::is_trivial_v<T>) {
			if (count == 1)
				*destination = *source;
			else
				__builtin_memmove(destination, source, count * sizeof(T));
			return count;
		}

		for (size_t i = 0; i < count; ++i) {
			if (destination <= source)
				new (&destination[i]) T(source[i]);
			else
				new (&destination[count - i - 1]) T(source[count - i - 1]);
		}

		return count;
	}

	static bool compare(T const* a, T const* b, size_t count)
	{
		if (count == 0)
			return true;

		if constexpr (std::is_trivial_v<T>)
			return !__builtin_memcmp(a, b, count * sizeof(T));

		for (size_t i = 0; i < count; ++i) {
			if (a[i] != b[i])
				return false;
		}

		return true;
	}

	static void delete_(T* ptr, size_t count)
	{
		if (count == 0)
			return;

		if constexpr (std::is_trivial_v<T>) {
			return;
		}

		for (size_t i = 0; i < count; ++i)
			ptr[i].~T();
	}
};

// end TypedTransfer.h

// Iterator.h

template <typename Container, typename ValueType>
class SimpleIterator {
public:
	friend Container;

	constexpr bool is_end() const { return m_index == SimpleIterator::end(m_container).m_index; }
	constexpr size_t index() const { return m_index; }

	constexpr bool operator==(SimpleIterator other) const { return m_index == other.m_index; }
	constexpr bool operator!=(SimpleIterator other) const { return m_index != other.m_index; }
	constexpr bool operator<(SimpleIterator other) const { return m_index < other.m_index; }
	constexpr bool operator>(SimpleIterator other) const { return m_index > other.m_index; }
	constexpr bool operator<=(SimpleIterator other) const { return m_index <= other.m_index; }
	constexpr bool operator>=(SimpleIterator other) const { return m_index >= other.m_index; }

	constexpr SimpleIterator operator+(ptrdiff_t delta) const { return SimpleIterator { m_container, m_index + delta }; }
	constexpr SimpleIterator operator-(ptrdiff_t delta) const { return SimpleIterator { m_container, m_index - delta }; }

	constexpr ptrdiff_t operator-(SimpleIterator other) const { return static_cast<ptrdiff_t>(m_index) - other.m_index; }

	constexpr SimpleIterator operator++()
	{
		++m_index;
		return *this;
	}
	constexpr SimpleIterator operator++(int)
	{
		++m_index;
		return SimpleIterator { m_container, m_index - 1 };
	}

	constexpr SimpleIterator operator--()
	{
		--m_index;
		return *this;
	}
	constexpr SimpleIterator operator--(int)
	{
		--m_index;
		return SimpleIterator { m_container, m_index + 1 };
	}

	ALWAYS_INLINE constexpr ValueType const& operator*() const { return m_container[m_index]; }
	ALWAYS_INLINE constexpr ValueType& operator*() { return m_container[m_index]; }

	ALWAYS_INLINE constexpr ValueType const* operator->() const { return &m_container[m_index]; }
	ALWAYS_INLINE constexpr ValueType* operator->() { return &m_container[m_index]; }

	SimpleIterator& operator=(SimpleIterator const& other)
	{
		m_index = other.m_index;
		return *this;
	}
	SimpleIterator(SimpleIterator const& obj) = default;

private:
	static constexpr SimpleIterator begin(Container& container) { return { container, 0 }; }
	static constexpr SimpleIterator end(Container& container)
	{
		using RawContainerType = std::remove_cv<Container>;

		return { container, container.size() };
	}

	constexpr SimpleIterator(Container& container, size_t index)
		: m_container(container)
		, m_index(index)
	{
	}

	Container& m_container;
	size_t m_index;
};

// end Iterator.h

namespace Detail {

template <typename T>
class Span {
public:
	ALWAYS_INLINE constexpr Span() = default;

	ALWAYS_INLINE constexpr Span(T* values, size_t size)
		: m_values(values)
		, m_size(size)
	{
	}

	template <size_t size>
	ALWAYS_INLINE constexpr Span(T (&values)[size])
		: m_values(values)
		, m_size(size)
	{
	}

	template <size_t size>
	ALWAYS_INLINE constexpr Span(std::array<T, size>& array)
		: m_values(array.data())
		, m_size(size)
	{
	}

	template <size_t size>
#if HAS_CPP20
	requires(std::is_const<T>)
#endif
		ALWAYS_INLINE constexpr Span(std::array<T, size> const& array)
		: m_values(const_cast<T*>(array.data()))
		, m_size(size)
	{
	}

	ALWAYS_INLINE Span(std::vector<T>& vector)
		: m_values(vector.data())
		, m_size(vector.size())
	{
	}

protected:
	T* m_values { nullptr };
	size_t m_size { 0 };
};

template <>
class Span<uint8_t> {
public:
	ALWAYS_INLINE constexpr Span() = default;

	ALWAYS_INLINE constexpr Span(uint8_t* values, size_t size)
		: m_values(values)
		, m_size(size)
	{
	}

	ALWAYS_INLINE Span(void* values, size_t size)
		: m_values(reinterpret_cast<uint8_t*>(values))
		, m_size(size)
	{
	}

	template <size_t size>
	ALWAYS_INLINE constexpr Span(uint8_t (&values)[size])
		: m_values(values)
		, m_size(size)
	{
	}

	ALWAYS_INLINE Span(std::vector<uint8_t>& vector)
		: m_values(vector.data())
		, m_size(vector.size())
	{
	}

protected:
	uint8_t* m_values { nullptr };
	size_t m_size { 0 };
};

template <>
class Span<uint8_t const> {
public:
	ALWAYS_INLINE constexpr Span() = default;

	ALWAYS_INLINE constexpr Span(uint8_t const* values, size_t size)
		: m_values(values)
		, m_size(size)
	{
	}

	ALWAYS_INLINE Span(void const* values, size_t size)
		: m_values(reinterpret_cast<uint8_t const*>(values))
		, m_size(size)
	{
	}

	ALWAYS_INLINE Span(char const* values, size_t size)
		: m_values(reinterpret_cast<uint8_t const*>(values))
		, m_size(size)
	{
	}

	template <size_t size>
	ALWAYS_INLINE constexpr Span(uint8_t const (&values)[size])
		: m_values(values)
		, m_size(size)
	{
	}

protected:
	uint8_t const* m_values { nullptr };
	size_t m_size { 0 };
};

}

template <typename T>
class Span : public Detail::Span<T> {
public:
	using Detail::Span<T>::Span;

	template <typename U>
	using ReadonlySpan = Span<U const>;

	constexpr Span() = default;

	[[nodiscard]] ALWAYS_INLINE constexpr T const* data() const { return this->m_values; }
	[[nodiscard]] ALWAYS_INLINE constexpr T* data() { return this->m_values; }

	[[nodiscard]] ALWAYS_INLINE constexpr T const* offset_pointer(size_t offset) const { return this->m_values + offset; }
	[[nodiscard]] ALWAYS_INLINE constexpr T* offset_pointer(size_t offset) { return this->m_values + offset; }

	using ConstIterator = SimpleIterator<Span const, T const>;
	using Iterator = SimpleIterator<Span, T>;

	constexpr ConstIterator begin() const { return ConstIterator::begin(*this); }
	constexpr Iterator begin() { return Iterator::begin(*this); }

	constexpr ConstIterator end() const { return ConstIterator::end(*this); }
	constexpr Iterator end() { return Iterator::end(*this); }

	[[nodiscard]] ALWAYS_INLINE constexpr size_t size() const { return this->m_size; }
	[[nodiscard]] ALWAYS_INLINE constexpr bool is_null() const { return this->m_values == nullptr; }
	[[nodiscard]] ALWAYS_INLINE constexpr bool is_empty() const { return this->m_size == 0; }

	[[nodiscard]] ALWAYS_INLINE constexpr Span slice(size_t start, size_t length) const
	{
		VERIFY(start + length <= size());
		return { this->m_values + start, length };
	}
	[[nodiscard]] ALWAYS_INLINE constexpr Span slice(size_t start) const
	{
		VERIFY(start <= size());
		return { this->m_values + start, size() - start };
	}
	[[nodiscard]] ALWAYS_INLINE constexpr Span slice_from_end(size_t count) const
	{
		VERIFY(count <= size());
		return { this->m_values + size() - count, count };
	}

	[[nodiscard]] ALWAYS_INLINE constexpr Span trim(size_t length) const
	{
		return { this->m_values, min(size(), length) };
	}

	[[nodiscard]] Span align_to(size_t alignment) const
	{
		auto* start = reinterpret_cast<T*>(align_up_to((FlatPtr)data(), alignment));
		auto* end = reinterpret_cast<T*>(align_down_to((FlatPtr)(data() + size()), alignment));
		if (end < start)
			return {};
		size_t length = end - start;
		return { start, length };
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T* offset(size_t start) const
	{
		VERIFY(start < this->m_size);
		return this->m_values + start;
	}

	ALWAYS_INLINE constexpr void overwrite(size_t offset, void const* data, size_t data_size)
	{
		// make sure we're not told to write past the end
		VERIFY(offset + data_size <= size());
		__builtin_memmove(this->data() + offset, data, data_size);
	}

	ALWAYS_INLINE constexpr size_t copy_to(Span<std::remove_const<T>> other) const
	{
		VERIFY(other.size() >= size());
		return TypedTransfer<std::remove_const<T>>::copy(other.data(), data(), size());
	}

	ALWAYS_INLINE constexpr size_t copy_trimmed_to(Span<std::remove_const<T>> other) const
	{
		auto const count = min(size(), other.size());
		return TypedTransfer<std::remove_const<T>>::copy(other.data(), data(), count);
	}

	ALWAYS_INLINE constexpr size_t fill(T const& value)
	{
		for (size_t idx = 0; idx < size(); ++idx)
			data()[idx] = value;

		return size();
	}

	[[nodiscard]] bool constexpr contains_slow(T const& value) const
	{
		for (size_t i = 0; i < size(); ++i) {
			if (at(i) == value)
				return true;
		}
		return false;
	}

	[[nodiscard]] bool constexpr starts_with(ReadonlySpan<T> other) const
	{
		if (size() < other.size())
			return false;

		return TypedTransfer<T>::compare(data(), other.data(), other.size());
	}

	[[nodiscard]] size_t constexpr matching_prefix_length(ReadonlySpan<T> other) const
	{
		auto maximum_length = min(size(), other.size());

		for (size_t i = 0; i < maximum_length; i++) {
			if (data()[i] != other.data()[i])
				return i;
		}

		return maximum_length;
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T const& at(size_t index) const
	{
		VERIFY(index < this->m_size);
		return this->m_values[index];
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T& at(size_t index)
	{
		VERIFY(index < this->m_size);
		return this->m_values[index];
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T const& first() const
	{
		return this->at(0);
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T& first()
	{
		return this->at(0);
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T const& last() const
	{
		return this->at(this->size() - 1);
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T& last()
	{
		return this->at(this->size() - 1);
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T const& operator[](size_t index) const
	{
		return at(index);
	}

	void reverse()
	{
		for (size_t i = 0; i < size() / 2; ++i)
			std::swap(at(i), at(size() - i - 1));
	}

	[[nodiscard]] ALWAYS_INLINE constexpr T& operator[](size_t index)
	{
		return at(index);
	}

	constexpr bool operator==(Span const& other) const
	{
		if (size() != other.size())
			return false;

		return TypedTransfer<T>::compare(data(), other.data(), size());
	}

	ALWAYS_INLINE constexpr operator ReadonlySpan<T>() const
	{
		return { data(), size() };
	}
};

template <typename T>
using ReadonlySpan = Span<T const>;

using ReadonlyBytes = ReadonlySpan<uint8_t>;
using Bytes = Span<uint8_t>;

template <typename T>
#if HAS_CPP20
requires(IsTrivial<T>)
#endif
	ReadonlyBytes to_readonly_bytes(Span<T> span)
{
	return ReadonlyBytes { static_cast<void const*>(span.data()), span.size() * sizeof(T) };
}

template <typename T>
#if HAS_CPP20
requires(IsTrivial<T> && !IsConst<T>)
#endif
	Bytes to_bytes(Span<T> span)
{
	return Bytes { static_cast<void*>(span.data()), span.size() * sizeof(T) };
}
