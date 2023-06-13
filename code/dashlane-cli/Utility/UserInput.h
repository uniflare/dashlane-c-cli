#pragma once

#include <iostream>

#include <strutil.h>

namespace Dashlane
{

	// TODO: Clean this lazy dirtyness
	struct HeadlessParameters
	{
		static std::string s_email;
		static std::string s_masterPassword;
		static std::string s_otpCode;
	};

	enum class EUserInputType
	{
		EmailToken,
		Login,
		MasterPassword,
		OTPCode
	};

	inline std::string GetUserInput(EUserInputType inputType)
	{
		std::string response;

		std::string message;
		switch (inputType)
		{
		case EUserInputType::Login:
			if (!HeadlessParameters::s_email.empty())
				return HeadlessParameters::s_email;

			message = "Please enter your email address: ";
			break;

		case EUserInputType::MasterPassword:
			if (!HeadlessParameters::s_masterPassword.empty())
				return HeadlessParameters::s_masterPassword;

			message = "Please enter your master password: ";
			break;

		case EUserInputType::EmailToken:
			message = "Please enter the email token that was delivered to your inbox: ";
			break;

		case EUserInputType::OTPCode:
			if (!HeadlessParameters::s_otpCode.empty())
				return HeadlessParameters::s_otpCode;

			message = "Please enter your two factor authentication code: ";
			break;
		}
		std::cout << message;
		std::cin >> response;

		return response;
	}

}