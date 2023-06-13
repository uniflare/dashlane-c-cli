#pragma once

#include <Utility/ConceptHelpers.h>
#include <stack>
#include <string>
#include <vector>

// Serialization Utility
// 
// This serialization utility is a much simplified and use-case specific serializer inspired by YASLI.
// 
// This means every object that you wish to serialize will have its own Serialize method right there in its
// definition. You can however, specify a serialization function outside the class if you wish. This function takes
// the object to serialize to/from as the second parameter.
// 
// The Serializer is designed to serialize to and from the format used by Dashlane - that is - flat, with $ as end of 
// object identifiers.
// 
// Caveats to usage:
//		1. When reading vectors, the size of the vector is used to gauge how many characters to consume. If the vector 
//		   size is 0, then the vector will consume the remaining data from the input buffer.
//		2. When SetSkipSeparator is set, all children will use this context and will skip reading/writing this end of 
//		   object identifier (only children, subsequent items on the same hierarchical level and above will go back to 
//		   the original context).

namespace Dashlane
{

	class CSerializer;

	namespace Detail
	{
		template<typename T>
		concept HasSerializeFunction = requires(CSerializer & ser, T obj)
		{
			{ Serialize(ser, obj) } -> std::convertible_to<bool>;
		};

		template<typename T>
		concept HasSerializeMethod = requires(CSerializer & ser, T obj)
		{
			{ obj.Serialize(ser) } -> std::convertible_to<bool>;
		};
	}

	class CSerializer
	{

	public:

		enum class EDirection
		{
			Out,
			In
		};

	private:

		struct SContext
		{
			bool skipSeparator{ false };
		};

		CSerializer(EDirection direction, std::vector<uint8_t>& buffer);
		CSerializer(CSerializer&&) = delete;
		CSerializer(const CSerializer&) = delete;

	public:

		bool IsInput();
		bool IsOutput();
		void SetSkipSeparators(bool skip);

		template <typename T>
		static bool DoSerialize(std::vector<uint8_t>& buffer, T& obj)
		{
			CSerializer serializer(EDirection::Out, buffer);
			return serializer(obj);
		}

		template <typename T>
		static bool DoDeserialize(const std::vector<uint8_t>& buffer, T& obj)
		{
			CSerializer serializer(EDirection::In, const_cast<std::vector<uint8_t>&>(buffer));
			return serializer(obj);
		}

		template<typename T>
			requires Detail::HasSerializeFunction<T> || Detail::HasSerializeMethod<T>
		bool operator()(T& obj)
		{
			m_contexts.emplace(std::move(SContext(m_nextContext)));

			const bool success = CallSerialize(obj);

			m_nextContext = m_contexts.top();
			m_contexts.pop();

			return success;
		}

		template<typename T>
			requires Detail::HasSerializeFunction<T> || Detail::HasSerializeMethod<T>
		bool operator()(const T & obj)
		{
			return (*this)(const_cast<T&>(obj));
		}

	protected:
		template<typename T>
			requires Detail::HasSerializeFunction<T> && (!Detail::HasSerializeMethod<T>)
		bool CallSerialize(T& obj)
		{
			return Serialize(*this, obj);
		}

		template<typename T>
			requires (!Detail::HasSerializeFunction<T>) && Detail::HasSerializeMethod<T>
		bool CallSerialize(T& obj)
		{
			return obj.Serialize(*this);
		}

		template <typename T>
			requires Utility::IterableSizeIsSame<T, uint8_t> && Utility::IterableConvertibleTo<T, uint8_t>
		void Write(const T& data)
		{
			if (data.size() > 0)
			{
				auto cast = reinterpret_cast<const uint8_t*>(data.data());
				m_pBuffer->insert(m_pBuffer->end(), cast, cast + data.size());
			}
			
			if (!m_contexts.top().skipSeparator)
			{
				m_pBuffer->push_back('$');
			}
		}

		template <typename T>
			requires Utility::IterableSizeIsSame<T, uint8_t> && Utility::IterableConvertibleTo<T, uint8_t>
		void Read(T& output, size_t length = 0)
		{
			if (m_bufferIter != m_pBuffer->cend())
			{
				auto endIter = m_pBuffer->cend();

				if (!m_contexts.top().skipSeparator && length == 0)
				{
					endIter = std::find(m_bufferIter, m_pBuffer->cend(), (uint8_t)'$');
				}
				else if (length > 0 && std::distance(m_pBuffer->cbegin(), m_bufferIter + length) <= m_pBuffer->size())
				{
					endIter = m_bufferIter + length;
				}

				if (endIter != m_bufferIter)
				{
					auto distance = std::distance(m_bufferIter, endIter);
					auto cast = reinterpret_cast<const uint8_t*>(&*m_bufferIter);
					output.resize(distance);
					std::memcpy(output.data(), cast, distance);
				}

				if (endIter != m_pBuffer->cend())
					m_bufferIter = m_contexts.top().skipSeparator ? endIter : ++endIter;
			}
		}

		template <typename T>
			requires std::integral<T> || std::floating_point<T>
		void Write(T number)
		{
			std::string s = std::to_string(number);
			Write(s);
		}

		friend bool Serialize(CSerializer& ser, std::string& s)
		{
			if (ser.IsInput())
				ser.Read(s);
			else
				ser.Write(s);

			return true;
		}

		template <typename T>
			requires std::integral<T> || std::floating_point<T>
		friend bool Serialize(CSerializer& ser, T& number)
		{
			if (ser.IsInput())
			{
				std::string token;
				ser.Read(token);
				std::from_chars_result result = std::from_chars(token.data(), token.data() + token.size(), number);
				return result.ptr == &(*token.rbegin());
			}
			else
				ser.Write(number);

			return true;
		}

		friend bool Serialize(CSerializer& ser, std::vector<uint8_t>& vec)
		{
			if (ser.IsInput())
				ser.Read(vec, vec.size());
			else
				ser.Write(vec);

			return true;
		}

	private:

		EDirection m_direction;
		std::vector<uint8_t>* m_pBuffer;
		std::vector<uint8_t>::const_iterator m_bufferIter;
		SContext m_nextContext;
		std::stack<SContext> m_contexts;

	};

}