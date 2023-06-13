#pragma once

namespace Utility
{

	namespace Detail
	{
		template<typename TElement>
		inline void StringJoin(std::string& str, const std::string& delimeter, const TElement& element)
		{
			str.append(delimeter);
			str.append(element);
		};

		template<typename TElement, typename...Elements>
		inline void StringJoin(std::string& str, const std::string& delimeter, const TElement& element, const Elements... elements)
		{
			str.append(delimeter);
			str.append(element);
			StringJoin(str, delimeter, std::forward<const Elements>(elements)...);
		};
	}

	template<typename TElement, typename...Elements>
	inline std::string StringJoin(const std::string& delimeter, const TElement& element, const Elements... elements)
	{
		std::string joined = element;
		Detail::StringJoin(joined, delimeter, std::forward<const Elements>(elements)...);
		return joined;
	};

	template<typename TElement>
	inline std::string StringJoinList(const std::string& delimeter, const std::span<const TElement>& elements)
	{
		std::string joined;

		auto it = elements.begin();
		if (it != elements.end())
		{
			joined = *it;

			while (++it != elements.end())
			{
				joined.append(delimeter);
				joined.append(*it);
			}
		}

		return joined;
	};

	template<typename TElement>
	inline std::string StringJoinList(const std::string& delimeter, const std::vector<TElement>& elements)
	{
		return StringJoinList(delimeter, std::span<const TElement>(elements.begin(), elements.end()));
	}

	inline std::string VectorU8ToString(const std::vector<uint8_t>& input)
	{
		return std::string(input.cbegin(), input.cend());
	}

	inline std::vector<uint8_t> StringToVectorU8(const std::string& input)
	{
		return std::vector<uint8_t>(input.cbegin(), input.cend());
	}

	template <class T>
		requires IterableSizeIsSame<T, uint8_t>&& IterableConvertibleTo<T, uint8_t>
	std::string ToHex(const T& input)
	{
		static constexpr char digits[] = "0123456789abcdef";
		std::string result;

		if (input.size() != 0)
		{
			result.reserve(input.size() * 2);

			for (const auto& byte : input)
			{
				const auto in = static_cast<const uint8_t>(byte);
				result.push_back(digits[in >> 4]);
				result.push_back(digits[in & 15]);
			}
		}

		return result;
	}

	template <class T>
		requires IterableSizeIsSame<T, uint8_t>&& IterableConvertibleTo<T, uint8_t>
	std::vector<uint8_t> FromHex(const T& input)
	{
		std::vector<uint8_t> result;

		if (input.size() != 0 && (input.size() & 1) == 0)
		{
			result.reserve(input.size() / 2);

			for (auto it = input.begin(); it < input.end(); it++)
			{
				const uint8_t left = static_cast<const uint8_t>(*it);
				const uint8_t right = static_cast<const uint8_t>(*++it);

				const uint8_t hi = (left <= '9') ? left - '0' : (left & 0x7) + 9;
				const uint8_t lo = (right <= '9') ? right - '0' : (right & 0x7) + 9;

				result.push_back((hi << 4) + lo);
			}
		}

		return result;
	}

}