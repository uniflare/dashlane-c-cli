#include <Application.h>
#include "Command.h"
#include "Common.h"

namespace Dashlane
{

	namespace
	{

		CLI::App* SyncCommand(CLI::App* pApp)
		{
			std::cout << "Executing sync" << std::endl;

			const std::string login = GetUserInput(EUserInputType::Login);

			CDashlaneContextWrapper dctx;
			ThrowOnError(dctx.Init(applicationName, login.c_str(), szAppAccessKey, szAppSecretKey));

			EDashlaneError rc = EDashlaneError::NoError;
			while (true)
			{
				rc = (EDashlaneError)Dash_SynchronizeVaultData(dctx.Get());
				if (!HandleReturnCode(dctx, rc))
					break;
			};

			ThrowOnError(rc);
			std::cout << "Successfully synchronized vault data" << std::endl;

			return nullptr;
		}

		void AdditionalRegistrator(CLI::App* pCommand)
		{
			pCommand->alias("s");
		}

		static auto _ = COMMAND(
			"sync", 
			"Manually synchronize the local vault with Dashlane", 
			&SyncCommand, 
			&AdditionalRegistrator
		);

	}

}