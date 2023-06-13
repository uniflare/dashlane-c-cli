#pragma once

#include <dashlane/dashlane.h>

#include <CLI/CLI.hpp>

namespace Dashlane
{
	class CCommand;

	class CApplication
	{
	public:
		CApplication();
		~CApplication();

		bool Initialize();
		int Run(int argc, char** argv);

	protected:

		const CCommand* GetCommandByName(const std::string& name) const;
		const std::string GetFullyQualifiedCommandName(CLI::App* pCommand) const;
		CLI::App* GetParentAppByChildName(const std::string& childName);

	private:
		CLI::App m_app;
	};

}