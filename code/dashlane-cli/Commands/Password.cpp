#include <Application.h>
#include "Command.h"
#include "Common.h"

#include <nlohmann/json.hpp>
#include <clip.h>

namespace Dashlane
{

	namespace
	{

		static auto s_pFilters = std::make_shared<std::vector<std::string>>();

		CLI::App* PasswordCommand(CLI::App* pApp)
		{
			const std::string login = GetUserInput(EUserInputType::Login);
			
			CDashlaneContextWrapper dctx;
			CQueryContextWrapper qctx;

			ThrowOnError(dctx.Init(applicationName, login.c_str(), szAppAccessKey, szAppSecretKey));
			ThrowOnError(qctx.Init());
			ThrowOnError(Dash_AddQueryTransactionTypes(qctx.Get(), (uint32_t)ETransactionType::Authentifiant));

			for (auto filter : ParseFilters(*s_pFilters))
				ThrowOnError(Dash_AddQueryFilter(qctx.Get(), filter.first.c_str(), filter.second.c_str()));

			std::vector<std::string> jsonData;
			ThrowOnError(Dash_SetQueryWriter(qctx.Get(), WriteQueryJson<decltype(jsonData)>, &jsonData));

			EDashlaneError rc = EDashlaneError::NoError;
			while (true)
			{
				rc = (EDashlaneError)Dash_QueryTransactions(dctx.Get(), qctx.Get());
				if (!HandleReturnCode(dctx, rc))
					break; 
			};

			if (rc == EDashlaneError::NoError)
			{
				if (jsonData.size() > 0)
				{
					const std::string output = pApp->get_option("--output")->as<std::string>();
					if (output == "json")
					{
						std::cout << "[" << std::endl;
						std::cout << strutil::join(jsonData, ",") << std::endl;
						std::cout << "]" << std::endl;
					}
					else
					{
						nlohmann::ordered_json json = nlohmann::ordered_json::parse(jsonData[0]);
						std::string password = json.find("Password").value().get<std::string>();
						if (!password.empty())
						if (output == "password")
						{
							std::cout << password << std::endl;
						}
						else
						{
							clip::set_text(password);
							std::cout << "Stored password in clipboard." << std::endl;
						}
					}
				}
			}
			ThrowOnError(rc);

			return nullptr;
		}

		void AdditionalRegistrator(CLI::App* pCommand)
		{
			pCommand->alias("p");

			// Options
			pCommand->add_option("--output", "How to print the passwords among `clipboard, password, json`. The JSON option outputs all the matching credentials")
				->default_str("clipboard");

			pCommand->add_option("filters...", *s_pFilters, "Filter credentials based on any parameter using <param>=<value>; if <param> is not specified in the filter, will default to url and title")
				->delimiter(';');
		}

		static auto _ = COMMAND(
			"password", 
			"Retrieve a password from the local vault and copy it to the clipboard", 
			&PasswordCommand, 
			&AdditionalRegistrator
		);

	}

}