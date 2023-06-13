#pragma once

namespace Utility
{

	template<typename T1, typename T2>
	inline void AppendToVector(std::vector<T1>& primary, const T2& v2)
	{
		primary.insert(primary.end(), v2.begin(), v2.end());
	}

	template<typename T1, typename T2, typename ...Args>
	inline void AppendToVector(std::vector<T1>& primary, const T2& v2, const Args... args)
	{
		primary.insert(primary.end(), v2.begin(), v2.end());
		AppendToVector(primary, std::forward<const Args>(args)...);
	}

	template <typename T, typename...Args>
	inline std::vector<T> CreateChainedVector(const std::vector<T>& v1, const Args...args)
	{
		std::vector<T> primary;
		AppendToVector(primary, v1, std::forward<const Args>(args)...);
		return primary;
	}

	// Returns a span of contiguous data.
	// If length is 0, the span will encompass all the remaining data after offset.
	template<typename T>
	inline std::span<T> SliceVectorBySpan(std::vector<T>& input, size_t offset = 0, size_t length = 0)
	{
		if (input.size() < offset + length)
		{
			throw(std::runtime_error("Tried to slice passed the end of input"));
		}

		if (length == 0)
		{
			length = input.size() - offset;
		}

		return std::span<T>{input.begin() + offset, input.begin() + offset + length};
	}

	// Returns a span of contiguous data.
	// If length is 0, the span will encompass all the remaining data after offset.
	template<typename T>
	inline const std::span<const T> SliceVectorBySpan(const std::vector<T>& input, size_t offset = 0, size_t length = 0)
	{
		if (input.size() < offset + length)
		{
			throw(std::runtime_error("Tried to slice passed the end of input"));
		}

		if (length == 0)
		{
			length = input.size() - offset;
		}

		return std::span<const T>{input.begin() + offset, input.begin() + offset + length};
	}

	// Returns a new vector of contiguous data.
	// If length is 0, the vector will encompass all the remaining data after offset.
	template<typename T>
	inline std::vector<T> SliceVectorByCopy(std::vector<T>& input, size_t offset = 0, size_t length = 0)
	{
		if (input.size() < offset + length)
		{
			throw(std::runtime_error("Tried to slice passed the end of input"));
		}

		if (length == 0)
		{
			length = input.size() - offset;
		}

		return std::vector<T>{input.begin() + offset, input.begin() + offset + length};
	}

	// Returns a new vector of contiguous data.
	// If length is 0, the vector will encompass all the remaining data after offset.
	template<typename T>
	inline std::vector<T> SliceVectorByCopy(const std::vector<T>& input, size_t offset = 0, size_t length = 0)
	{
		if (input.size() < offset + length)
		{
			throw(std::runtime_error("Tried to slice passed the end of input"));
		}

		if (length == 0)
		{
			length = input.size() - offset;
		}

		return std::vector<T>{input.begin() + offset, input.begin() + offset + length};
	}

}