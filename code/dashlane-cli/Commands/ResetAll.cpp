#include <Application.h>
#include "Command.h"
#include "Common.h"

namespace Dashlane
{

	namespace
	{

		CLI::App* ResetAllCommand(CLI::App* pApp)
		{
			std::string input;
			std::cout << "Are you sure you want to reset all user vaults? (y/n): ";
			std::cin >> input;

			if (input == "y" || input == "Y")
			{
				const std::string login = GetUserInput(EUserInputType::Login);

				CDashlaneContextWrapper dctx;
				ThrowOnError(dctx.Init(applicationName, login.c_str(), szAppAccessKey, szAppSecretKey));
				ThrowOnError(Dash_ResetVaultData(dctx.Get(), true));

				std::cout << "Successfully reset vault all data" << std::endl;
			}

			return nullptr;
		}

		static auto _ = COMMAND(
			"resetall",
			"Reset and clean your local database and OS keychain for all users",
			&ResetAllCommand
		);

	}

}