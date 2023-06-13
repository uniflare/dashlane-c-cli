#pragma once

#include "../endpoints/performTotpVerification.h"
#include "../endpoints/completeLoginWithAuthTicket.h"
#include "../endpoints/get2FAStatusUnauthenticated.h"

#include <iostream>

namespace dlib
{
    struct Params
    {
        std::string login;
        std::string deviceAccessKey;
    };

    inline std::string perform2FAVerification(const Params& params)
    {
        std::string serverKey;

        Get2FAStatusOutput twoFactorAuthStatus = get2FAStatusUnauthenticated({ params.login });
        switch (twoFactorAuthStatus.type)
        {

        case E2FAType::TotpLogin:
        {
            std::cout << "OTP Code: ";
            std::string otp;
            std::cin >> otp;
            std::cout << std::endl;

            const std::string authTicket = performTotpVerification({ params.login, otp }).authTicket;

            auto response = completeLoginWithAuthTicket({ params.login, authTicket, params.deviceAccessKey });

            if (!response.ssoServerKey.empty())
            {
                throw(std::runtime_error("SSO Authentication not supported"));
            }

            serverKey = response.serverKey;
        }

        }

        return serverKey;
    }
}