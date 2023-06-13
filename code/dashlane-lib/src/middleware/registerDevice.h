#pragma once

#include "../endpoints/completeDeviceRegistration.h"
#include "../endpoints/performDashlaneAuthenticatorVerification.h"
#include "../endpoints/performDuoPushVerification.h"
#include "../endpoints/performEmailTokenVerification.h"
#include "../endpoints/performTotpVerification.h"
#include "../endpoints/getAuthenticationMethodsForDevice.h"
#include "../endpoints/requestEmailTokenVerification.h"

namespace dlib
{

    CompleteDeviceRegistrationWithAuthTicketOutput registerDevice(const std::string& login)
	{
		const std::vector<std::unique_ptr<AuthenticationMethodBase>> verifications =  getAuthenticationMethodsForDevice(login);

		std::string authTicket;
        for (const std::unique_ptr<AuthenticationMethodBase>& authMethodBase : verifications)
        {
            switch (authMethodBase->type)
            {

            case EAuthMethod::DuoPush:
                authTicket = performDuoPushVerification({ login }).authTicket;
                break;

            case EAuthMethod::DashlaneAuthenticator:
                authTicket = performDashlaneAuthenticatorVerification({ login }).authTicket;
                break;

            case EAuthMethod::Totp:
            {
                std::cout << "OTP Code: ";
                std::string otp;
                std::cin >> otp;
                std::cout << std::endl;
                authTicket = performTotpVerification({ login, otp }).authTicket;
            } break;

            case EAuthMethod::EmailToken:
            {
                requestEmailTokenVerification({ login });
                std::cout << "Email Token: ";
                std::string token;
                std::cin >> token;
                std::cout << std::endl;
                authTicket = performEmailTokenVerification({ login, token }).authTicket;
            } break;

            }
        }

        if (authTicket.empty())
        {
            throw(std::runtime_error("Error: Auth verification method failed or is not supported"));
        }

        return completeDeviceRegistration({ login, authTicket });
    }
}