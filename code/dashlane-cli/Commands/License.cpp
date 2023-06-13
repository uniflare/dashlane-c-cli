#include "Command.h"
#include <License-Gen.h>

namespace Dashlane
{

	namespace
	{

		CLI::App* LicenseCommand(CLI::App* pApp)
		{
			std::cout << g_licenseText;
			return nullptr;
		}

		static auto _ = COMMAND(
			"license",
			"Output the licenses associated with this application",
			&LicenseCommand
		);

	}

}