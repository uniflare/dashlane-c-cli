//
//  EnumClass.h
//  ArticleEnumClass
//
//  Created by Gabriel Aubut-Lussier on 17-08-07.
//  Copyright © 2017 Gabriel Aubut-Lussier. All rights reserved.
//
//  Modified by uniflare on 11-06-23.
//   - Added iterators
//   - Added all() and none() methods
// 
//  BSD 2-Clause License
//  
//  Copyright (c) 2017, Gabriel Aubut-Lussier
//  All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//  
//  * Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//  
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef EnumClass_h
#define EnumClass_h

#include <cassert>
#include <type_traits>

template<typename T>
struct enable_enum_class_bitmask
{
	static constexpr bool value = false;
	static constexpr size_t bits = 0;
};

#define DECLARE_ENUM_BITMASK(Type, Size)																 \
template<> struct enable_enum_class_bitmask<Type> 														 \
{ 																										 \
	static_assert(static_cast<size_t>(Size) < 64, "Maximum size of custom bitsets are 63 usable bits."); \
	static constexpr bool value = true; 																 \
	static constexpr size_t bits = static_cast<size_t>(Size); 											 \
}

/**
 * Wrapper for an enumerator that provides implicit bool conversion
 */
template <typename T>
struct enumerator
{
	constexpr enumerator(const T& value) : value(value) {}
	constexpr explicit operator bool() const
	{
		using underlying_type = typename std::underlying_type<T>::type;
		return static_cast<underlying_type>(value) != 0;
	}
	constexpr operator T() const
	{
		return value;
	}
	
	T value;
};

/**
 * Wrapper that differentiates combined enumerators from a single enumerator
 * to prevent accidental comparisons between a bitmask and a single enumerator
 * using operator== or operator!=
 */
template <typename T>
struct bitmask
{
	using underlying_type = typename std::underlying_type<T>::type;
	static constexpr size_t max_bit_value = 1 << enable_enum_class_bitmask<T>::bits;
	static constexpr size_t end_bit_value = max_bit_value << 1;
	
	constexpr bitmask(const T& value) : value(static_cast<underlying_type>(value)) {}
	constexpr bitmask(const enumerator<T>& enumerator) : value(static_cast<underlying_type>(enumerator.value)) {}
	constexpr explicit operator bool() const
	{
		return value != 0;
	}

	static constexpr bitmask<T> all() { return (T)(std::numeric_limits<underlying_type>::max() >> ((8*sizeof T) - enable_enum_class_bitmask<T>::bits)); }
	static constexpr bitmask<T> none() { return (T)underlying_type(0); }

	struct const_iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = underlying_type;
		using value_type = enumerator<T>;

		const_iterator(underlying_type value, size_t shift)
			: m_value(value)
			, m_shift(shift)
		{
			if (m_shift == 0 && !(m_value & (1 << m_shift)))
			{
				++(*this);
			}
		}

		enumerator<T> operator*() const { return (T)((size_t)1 << m_shift); }

		const_iterator& operator++()
		{
			while (((size_t)1 << ++m_shift) < end_bit_value)
			{
				if (((size_t)1 << m_shift) & m_value)
				{
					break;
				}
			}

			return *this;
		}

		const_iterator operator++(int) const { const_iterator tmp = *this; ++(*this); return tmp; }

		friend bool operator== (const const_iterator& a, const const_iterator& b) { return *a == *b; };
		friend bool operator!= (const const_iterator& a, const const_iterator& b) { return *a != *b; };

		size_t m_shift;
		underlying_type m_value;
	};

	const_iterator begin() const { return const_iterator(value, 0); }
	const_iterator end() const { return const_iterator(value, enable_enum_class_bitmask<T>::bits+1); }
	
	underlying_type value;
};

template<typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
make_bitmask(const T& t)
{
	return bitmask<T>{t};
}

/**
 * operator&(T, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, enumerator<T>>::type
operator&(const T& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	assert((static_cast<underlying_type>(lhs) & (static_cast<underlying_type>(lhs) - 1)) == 0);
	return enumerator<T>{static_cast<T>(static_cast<underlying_type>(lhs) & static_cast<underlying_type>(rhs))};
}

/**
 * operator&(enumerator<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, enumerator<T>>::type
operator&(const enumerator<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return enumerator<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) & static_cast<underlying_type>(rhs.value))};
}

/**
 * operator&(bitmask<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator&(const bitmask<T>& lhs, const bitmask<T>& rhs)
{
	return bitmask<T>{static_cast<T>(lhs.value & rhs.value)};
}

/**
 * operator&(bitmask<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, enumerator<T>>::type
operator&(const bitmask<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return enumerator<T>{static_cast<T>(lhs.value & static_cast<underlying_type>(rhs))};
}

/**
 * operator&(T, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, enumerator<T>>::type
operator&(const T& lhs, const bitmask<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return enumerator<T>{static_cast<T>(static_cast<underlying_type>(lhs) & rhs.value)};
}

/**
 * operator&(bitmask<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, enumerator<T>>::type
operator&(const bitmask<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return enumerator<T>{static_cast<T>(lhs.value & static_cast<underlying_type>(rhs.value))};
}

/**
 * operator&(enumerator<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, enumerator<T>>::type
operator&(const enumerator<T>& lhs, const bitmask<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return enumerator<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) & rhs.value)};
}

/**
 * operator&(T, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, enumerator<T>>::type
operator&(const T& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return enumerator<T>{static_cast<T>(static_cast<underlying_type>(lhs) & static_cast<underlying_type>(rhs.value))};
}

/**
 * operator&(enumerator<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, enumerator<T>>::type
operator&(const enumerator<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return enumerator<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) & static_cast<underlying_type>(rhs))};
}

/**
 * operator|(T, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const T& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs) | static_cast<underlying_type>(rhs))};
}

/**
 * operator|(enumerator<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const enumerator<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) | static_cast<underlying_type>(rhs.value))};
}

/**
 * operator|(bitmask<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const bitmask<T>& lhs, const bitmask<T>& rhs)
{
	return bitmask<T>{static_cast<T>(lhs.value | rhs.value)};
}

/**
 * operator|(bitmask<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const bitmask<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(lhs.value | static_cast<underlying_type>(rhs))};
}

/**
 * operator|(T, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const T& lhs, const bitmask<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs) | rhs.value)};
}

/**
 * operator|(bitmask<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const bitmask<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(lhs.value | static_cast<underlying_type>(rhs.value))};
}

/**
 * operator|(enumerator<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const enumerator<T>& lhs, const bitmask<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) | rhs.value)};
}

/**
 * operator|(enumerator<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const enumerator<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) | static_cast<underlying_type>(rhs))};
}

/**
 * operator|(T, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator|(const T& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs) | static_cast<underlying_type>(rhs.value))};
}

/**
 * operator^(T, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const T& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs) ^ static_cast<underlying_type>(rhs))};
}

/**
 * operator^(enumerator<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const enumerator<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) ^ static_cast<underlying_type>(rhs.value))};
}

/**
 * operator^(bitmask<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const bitmask<T>& lhs, const bitmask<T>& rhs)
{
	return bitmask<T>{static_cast<T>(lhs.value ^ rhs.value)};
}

/**
 * operator^(bitmask<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const bitmask<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(lhs.value ^ static_cast<underlying_type>(rhs))};
}

/**
 * operator^(T, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const T& lhs, const bitmask<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs) ^ rhs.value)};
}

/**
 * operator^(bitmask<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const bitmask<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(lhs.value ^ static_cast<underlying_type>(rhs.value))};
}

/**
 * operator^(enumerator<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const enumerator<T>& lhs, const bitmask<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) ^ rhs.value)};
}

/**
 * operator^(enumerator<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const enumerator<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs.value) ^ static_cast<underlying_type>(rhs))};
}

/**
 * operator^(T, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator^(const T& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(static_cast<underlying_type>(lhs) ^ static_cast<underlying_type>(rhs.value))};
}

/**
 * operator~(T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator~(const T& value)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(~ static_cast<underlying_type>(value))};
}

/**
 * operator~(enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator~(const enumerator<T>& lhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	return bitmask<T>{static_cast<T>(~ static_cast<underlying_type>(lhs.value))};
}

/**
 * operator~(bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>>::type
operator~(const bitmask<T>& lhs)
{
	return bitmask<T>{static_cast<T>(~ lhs.value)};
}

/**
 * operator&=(bitmask<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator&=(bitmask<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	lhs.value &= static_cast<underlying_type>(rhs);
	return lhs;
}

/**
 * operator&=(bitmask<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator&=(bitmask<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	lhs.value &= static_cast<underlying_type>(rhs.value);
	return lhs;
}

/**
 * operator&=(bitmask<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator&=(bitmask<T>& lhs, const bitmask<T>& rhs)
{
	lhs.value &= rhs.value;
	return lhs;
}

/**
 * operator|=(bitmask<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator|=(bitmask<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	lhs.value |= static_cast<underlying_type>(rhs);
	return lhs;
}

/**
 * operator|=(bitmask<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator|=(bitmask<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	lhs.value |= static_cast<underlying_type>(rhs.value);
	return lhs;
}

/**
 * operator|=(bitmask<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator|=(bitmask<T>& lhs, const bitmask<T>& rhs)
{
	lhs.value |= rhs.value;
	return lhs;
}

/**
 * operator^=(bitmask<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator^=(bitmask<T>& lhs, const T& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	lhs.value ^= static_cast<underlying_type>(rhs);
	return lhs;
}

/**
 * operator^=(bitmask<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator^=(bitmask<T>& lhs, const enumerator<T>& rhs)
{
	using underlying_type = typename std::underlying_type<T>::type;
	lhs.value ^= static_cast<underlying_type>(rhs.value);
	return lhs;
}

/**
 * operator^=(bitmask<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bitmask<T>&>::type
operator^=(bitmask<T>& lhs, const bitmask<T>& rhs)
{
	lhs.value ^= rhs.value;
	return lhs;
}

/**
 * operator==(T, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const T& lhs, const T& rhs)
{
	return lhs == rhs;
}

/**
 * operator==(enumerator<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const enumerator<T>& lhs, const enumerator<T>& rhs)
{
	return lhs.value == rhs.value;
}

/**
 * operator==(bitmask<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const bitmask<T>& lhs, const bitmask<T>& rhs)
{
	return lhs.value == rhs.value;
}

/**
 * operator==(enumerator<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const enumerator<T>& lhs, const T& rhs)
{
	return lhs.value == rhs;
}

/**
 * operator==(T, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const T& lhs, const enumerator<T>& rhs)
{
	return lhs == rhs.value;
}

/**
 * operator==(bitmask<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const bitmask<T>& lhs, const T& rhs)
{
	static_assert(!std::is_same<typename bitmask<T>::underlying_type, typename std::underlying_type<T>::type>::value, "A bitmask can't be compared to an enumerator. Use & first.");
	return false;
}

/**
 * operator==(T, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const T& lhs, const bitmask<T>& rhs)
{
	static_assert(!std::is_same<typename bitmask<T>::underlying_type, typename std::underlying_type<T>::type>::value, "A bitmask can't be compared to an enumerator. Use & first.");
	return false;
}

/**
 * operator==(bitmask<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const bitmask<T>& lhs, const enumerator<T>& rhs)
{
	static_assert(!std::is_same<typename bitmask<T>::underlying_type, typename std::underlying_type<T>::type>::value, "A bitmask can't be compared to an enumerator. Use & first.");
	return false;
}

/**
 * operator==(enumerator<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator==(const enumerator<T>& lhs, const bitmask<T>& rhs)
{
	static_assert(!std::is_same<typename bitmask<T>::underlying_type, typename std::underlying_type<T>::type>::value, "A bitmask can't be compared to an enumerator. Use & first.");
	return false;
}

/**
 * operator!=(T, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const T& lhs, const T& rhs)
{
	return lhs != rhs;
}

/**
 * operator!=(enumerator<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const enumerator<T>& lhs, const enumerator<T>& rhs)
{
	return lhs.value != rhs.value;
}

/**
 * operator!=(bitmask<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const bitmask<T>& lhs, const bitmask<T>& rhs)
{
	return lhs.value != rhs.value;
}

/**
 * operator!=(enumerator<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const enumerator<T>& lhs, const T& rhs)
{
	return lhs.value != rhs;
}

/**
 * operator!=(T, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const T& lhs, const enumerator<T>& rhs)
{
	return lhs != rhs.value;
}

/**
 * operator!=(bitmask<T>, T)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const bitmask<T>& lhs, const T& rhs)
{
	static_assert(!std::is_same<typename bitmask<T>::underlying_type, typename std::underlying_type<T>::type>::value, "A bitmask can't be compared to an enumerator. Use & first.");
	return false;
}

/**
 * operator!=(T, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const T& lhs, const bitmask<T>& rhs)
{
	static_assert(!std::is_same<typename bitmask<T>::underlying_type, typename std::underlying_type<T>::type>::value, "A bitmask can't be compared to an enumerator. Use & first.");
	return false;
}

/**
 * operator!=(bitmask<T>, enumerator<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const bitmask<T>& lhs, const enumerator<T>& rhs)
{
	static_assert(!std::is_same<typename bitmask<T>::underlying_type, typename std::underlying_type<T>::type>::value, "A bitmask can't be compared to an enumerator. Use & first.");
	return false;
}

/**
 * operator!=(enumerator<T>, bitmask<T>)
 */
template <typename T>
constexpr
typename std::enable_if<std::is_enum<T>::value && enable_enum_class_bitmask<T>::value, bool>::type
operator!=(const enumerator<T>& lhs, const bitmask<T>& rhs)
{
	static_assert(!std::is_same<typename bitmask<T>::underlying_type, typename std::underlying_type<T>::type>::value, "A bitmask can't be compared to an enumerator. Use & first.");
	return false;
}

#endif /* EnumClass_h */
