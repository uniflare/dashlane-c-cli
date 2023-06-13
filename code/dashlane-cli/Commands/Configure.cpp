#include <Application.h>
#include "Command.h"
#include "Common.h"

namespace Dashlane
{

	namespace
	{

		CLI::App* EnableAutoSyncCommand(CLI::App* pApp)
		{
			const std::string login = GetUserInput(EUserInputType::Login);

			CDashlaneContextWrapper dctx;
			ThrowOnError(dctx.Init(applicationName, login.c_str(), szAppAccessKey, szAppSecretKey));
			ThrowOnError(Dash_SetAutoSync(dctx.Get(), true));

			std::cout << "Successfully enabled auto-sync" << std::endl;

			return nullptr;
		}

		CLI::App* DisableAutoSyncCommand(CLI::App* pApp)
		{
			const std::string login = GetUserInput(EUserInputType::Login);

			CDashlaneContextWrapper dctx;
			ThrowOnError(dctx.Init(applicationName, login.c_str(), szAppAccessKey, szAppSecretKey));
			ThrowOnError(Dash_SetAutoSync(dctx.Get(), false));

			std::cout << "Successfully disabled auto-sync" << std::endl;

			return nullptr;
		}

		CLI::App* SaveMasterPasswordCommand(CLI::App* pApp)
		{
			const std::string login = GetUserInput(EUserInputType::Login);

			CDashlaneContextWrapper dctx;
			ThrowOnError(dctx.Init(applicationName, login.c_str(), szAppAccessKey, szAppSecretKey));
			ThrowOnError(Dash_SetShouldStoreMasterPassword(dctx.Get(), true));

			std::cout << "Successfully configured save-master-password preference" << std::endl;

			return nullptr;
		}

		CLI::App* NoSaveMasterPasswordCommand(CLI::App* pApp)
		{
			const std::string login = GetUserInput(EUserInputType::Login);

			CDashlaneContextWrapper dctx;
			ThrowOnError(dctx.Init(applicationName, login.c_str(), szAppAccessKey, szAppSecretKey));
			ThrowOnError(Dash_SetShouldStoreMasterPassword(dctx.Get(), false));

			std::cout << "Successfully configured no-save-master-password preference" << std::endl;

			return nullptr;
		}

		CLI::App* ConfigureCommand(CLI::App* pApp)
		{
			std::vector<CLI::App*> subcommands = pApp->get_subcommands();

			if (subcommands.size() > 1)
				throw(CLI::ParseError("Only one sub-command can be specified on the command-line", -1));

			return subcommands[0];
		}

		void AdditionalConfigureRegistrator(CLI::App* pCommand)
		{
			pCommand->alias("c");
			pCommand->require_subcommand();
		}

		static auto _ = COMMAND("configure", "Configure the CLI", &ConfigureCommand, &AdditionalConfigureRegistrator)
			.COMMAND("enable-auto-sync", "Enable automatic synchronization which is done once per hour", &EnableAutoSyncCommand)
			.COMMAND("disable-auto-sync", "Disable automatic synchronization which is done once per hour", &DisableAutoSyncCommand)
			.COMMAND("save-master-password", "Save the encrypted master password", &SaveMasterPasswordCommand)
			.COMMAND("no-save-master-password", "Do not save the encrypted master password", &NoSaveMasterPasswordCommand);

	}

}