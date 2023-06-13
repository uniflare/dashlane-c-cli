#include "StdAfx.h"
#include "Serialization.h"

namespace Dashlane
{

	CSerializer::CSerializer(EDirection direction, std::vector<uint8_t>& buffer)
		: m_direction(direction)
		, m_pBuffer(&buffer)
		, m_bufferIter()
	{
		if (direction == EDirection::In)
		{
			m_bufferIter = buffer.begin();
		}
	}

	bool CSerializer::IsInput()
	{
		return m_direction == EDirection::In;
	}

	bool CSerializer::IsOutput()
	{
		return m_direction == EDirection::Out;
	}

	void CSerializer::SetSkipSeparators(bool skip)
	{
		m_nextContext.skipSeparator = skip;
	}

}