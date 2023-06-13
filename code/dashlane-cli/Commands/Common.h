#pragma once

#include <CoreConfig.h>
#include <Utility/DashlaneContext.h>
#include <Utility/UserInput.h>

#include <strutil.h>

namespace Dashlane
{

	inline bool HandleReturnCode(CDashlaneContextWrapper& dctx, EDashlaneError rc)
	{
		switch (rc)
		{
		case EDashlaneError::RequireMasterPassword:
		{
			do
			{
				std::string masterPassword = GetUserInput(EUserInputType::MasterPassword);
				rc = (EDashlaneError)Dash_AssignMasterPassword(dctx.Get(), masterPassword.c_str());
			} while (rc != EDashlaneError::NoError);
		} break;

		case EDashlaneError::RequireEmailToken:
		{
			do
			{
				std::string emailToken = GetUserInput(EUserInputType::EmailToken);
				rc = (EDashlaneError)Dash_AssignEmailToken(dctx.Get(), emailToken.c_str());
			} while (rc != EDashlaneError::NoError);
		} break;

		case EDashlaneError::Require2FACode:
		{
			do
			{
				std::string otpCode = GetUserInput(EUserInputType::OTPCode);
				rc = (EDashlaneError)Dash_Assign2FACode(dctx.Get(), otpCode.c_str());
			} while (rc != EDashlaneError::NoError);
		} break;

		case EDashlaneError::InvalidMasterPassword:
		{
			std::cerr << "Master password is not valid" << std::endl;
		} break;

		case EDashlaneError::InvalidEmailToken:
		{
			std::cerr << "Email token is not valid" << std::endl;
		} break;

		case EDashlaneError::Invalid2FACode:
		{
			std::cerr << "2FA Code is not valid" << std::endl;
		} break;

		case EDashlaneError::NoError:
			return false;

		default:
			std::cerr << Dash_GetErrorMessage((uint32_t)rc) << std::endl;
			return false;
		}

		return true;
	}

	template <typename T>
		requires std::is_same_v<T, EDashlaneError> || std::is_same_v<T, uint32_t>
	inline void ThrowOnError(const T rc)
	{
		if ((EDashlaneError)rc != EDashlaneError::NoError)
		{
			std::cerr << Dash_GetErrorMessage((uint32_t)rc) << std::endl;
			throw CLI::RuntimeError((int)rc);
		}
	}

	inline std::map<std::string, std::string> ParseFilters(std::vector<std::string>& filters)
	{
		std::map<std::string, std::string> map;

		for (auto filter : filters)
		{
			auto split = strutil::split(filter, '=');
			if (split.size() == 2)
			{
				map[split[0]] = split[1];
			}
			else if (split.size() == 1)
			{
				map[split[0]] = "";
			}
		}

		return map;
	}

	template<typename T>
	inline void WriteQueryJson(void* pUserPointer, const char* json, uint32_t size)
	{
		T& buffer = *reinterpret_cast<T*>(pUserPointer);
		buffer.emplace_back(json);
	}

}