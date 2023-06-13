#pragma once

#include <type_traits>

namespace Utility
{

	template <typename T>
	using IteratorValueType = std::iterator_traits<typename T::iterator>::value_type;

	template<typename Iterable, typename SizeType>
	concept IterableSizeIsSame = sizeof IteratorValueType<Iterable> == sizeof SizeType;

	template<typename Iterable, typename To>
	concept IterableConvertibleTo = std::is_convertible<IteratorValueType<Iterable>, To>::value;

}