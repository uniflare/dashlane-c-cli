#include "Application.h"
#include "CommandRegistry.h"
#include "Commands/Command.h"
#include "Utility/UserInput.h"

#include <strutil.h>

namespace Dashlane
{
	std::string HeadlessParameters::s_email = "";
	std::string HeadlessParameters::s_masterPassword = "";
	std::string HeadlessParameters::s_otpCode = "";

	CApplication::CApplication()
		: m_app{ "Dashlane C++ CLI Interface by uniflare (Based on Dashlane's Command Line Interface project)" }
	{}

	CApplication::~CApplication()
	{}

	bool CApplication::Initialize()
	{
		m_app.require_subcommand();
		CCommandRegistry::VisitCommands([this](const CCommand& cmd)
		{
			// Get the parent App pointer if this is a child command
			CLI::App* pParentCommand = GetParentAppByChildName(cmd.GetName());

			// Get the child command name, from the fully qualified name
			std::string commandName = cmd.GetName();
			if (strutil::contains(commandName, "::"))
			{
				auto split = strutil::split(commandName, "::");
				commandName = split[split.size() - 1];
			}

			CLI::App* pCommand = pParentCommand->add_subcommand(commandName, cmd.GetDesc());
			const CommandRegistrator& pAdditionalRegistrator = cmd.GetAdditionalRegistrator();
			if (pAdditionalRegistrator)
			{
				pAdditionalRegistrator(pCommand);
			}
			return EVisitorState::Continue;
		});

		// Add global options
		m_app.add_option_function<std::string>("--masterpass", [](const std::string& masterPassword) 
		{
			HeadlessParameters::s_masterPassword = masterPassword;
		}, "Pre-set the master password prior to invocation.");

		m_app.add_option_function<std::string>("--email", [](const std::string& email)
		{
			HeadlessParameters::s_email = email;
		}, "Pre-set the email prior to invocation.");

		m_app.add_option_function<std::string>("--otp", [](const std::string& otpCode)
		{
			HeadlessParameters::s_otpCode = otpCode;
		}, "Pre-set the OTP code prior to invocation.");

		return true;
	}

	int CApplication::Run(int argc, char** argv)
	{
		// Parse command-line
		std::vector<CLI::App*> subcommands;
		try
		{
			m_app.parse(argc, argv);
			subcommands = m_app.get_subcommands();
			if (subcommands.size() > 1)
			{
				throw(CLI::ParseError("Only one command can be specified on the command-line", -1));
			}
		}
		catch (const CLI::ParseError& e)
		{
			return m_app.exit(e);
		}

		// Traverse and execute command hierarchy
		try
		{
			CLI::App* pSubCommand = subcommands[0];
			while (pSubCommand != nullptr)
			{
				const CCommand* pCommand = GetCommandByName(GetFullyQualifiedCommandName(pSubCommand));
				if (pCommand != nullptr)
					pSubCommand = (*pCommand)(pSubCommand);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "Error: " << e.what() << std::endl;
			return 1;
		}

		return 0;
	}

	const CCommand* CApplication::GetCommandByName(const std::string& name) const
	{
		const CCommand* pCommand = nullptr;

		CCommandRegistry::VisitCommands([&name, &pCommand](const CCommand& cmd)
		{
			if (name == cmd.GetName())
			{
				pCommand = &cmd;
				return EVisitorState::Stop;
			}
			return EVisitorState::Continue;
		});

		return pCommand;
	}

	const std::string CApplication::GetFullyQualifiedCommandName(CLI::App* pCommand) const
	{
		std::vector<std::string> namesReversed;

		CLI::App* pParent = pCommand;
		while (pParent != nullptr && pParent != &m_app)
		{
			namesReversed.emplace_back(pParent->get_name());
			pParent = pParent->get_parent();
		}

		std::string fullName = *namesReversed.crbegin();
		for (auto it = namesReversed.crbegin()+1; it != namesReversed.crend(); it++)
		{
			fullName += "::";
			fullName += *it;
		}
		return fullName;
	}

	CLI::App* CApplication::GetParentAppByChildName(const std::string& childName)
	{
		std::vector<std::string> names = strutil::split(childName, "::");

		CLI::App* pParentApp = &m_app;
		for (const std::string& name : names)
		{
			if (name == *names.crbegin())
			{
				break;
			}

			for (CLI::App* pApp : pParentApp->get_subcommands(nullptr))
			{
				if (pApp->get_name() == name)
				{
					pParentApp = pApp;
					break;
				}
			}
		}

		return pParentApp;
	}

}