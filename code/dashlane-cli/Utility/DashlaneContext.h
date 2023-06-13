#pragma once

#include <dashlane/dashlane.h>

namespace Dashlane
{

	class CDashlaneContextWrapper
	{

	public:

		CDashlaneContextWrapper() 
			: m_pContext(nullptr)
		{}

		~CDashlaneContextWrapper()
		{
			if (m_pContext != nullptr) Dash_FreeContext(m_pContext);
		}

		EDashlaneError Init(const char* szApplicationName, const char* szLogin, const char* szAppAccessKey, const char* szAppSecretKey)
		{
			return (EDashlaneError)Dash_InitContext(&m_pContext, szApplicationName, szLogin, szAppAccessKey, szAppSecretKey);
		}

		DashlaneContext* Get() const { return m_pContext; }

	private:

		DashlaneContext* m_pContext;

	};

	class CQueryContextWrapper
	{

	public:

		CQueryContextWrapper() 
			: m_pContext(nullptr)
		{}
		~CQueryContextWrapper()
		{
			if (m_pContext != nullptr)
				Dash_FreeQueryContext(m_pContext);
		}

		EDashlaneError Init()
		{
			return (EDashlaneError)Dash_InitQueryContext(&m_pContext);
		}

		DashlaneQueryContext* Get() const { return m_pContext; }

	private:

		DashlaneQueryContext* m_pContext;

	};

}