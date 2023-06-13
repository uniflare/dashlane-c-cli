#include "CommandRegistry.h"
#include <Commands/Command.h>

namespace Dashlane
{

	std::vector<CCommand> CCommandRegistry::s_registry = {};

	void CCommandRegistry::Register(CCommand&& cmd)
	{
		s_registry.emplace_back(std::move(cmd));
	}

	void CCommandRegistry::VisitCommands(std::function<EVisitorState(const CCommand&)> visitor)
	{
		for (auto& command : s_registry)
		{
			const EVisitorState state = visitor(command);
			if (state == EVisitorState::Stop)
				break;
		}
	}

	CSubRegistrator COMMAND(const std::string& name, std::string&& desc, CLICommand&& cmd, CommandRegistrator additionalRegistrator)
	{
		CCommandRegistry::Register(
			CCommand(name, std::move(desc), std::move(cmd), std::move(additionalRegistrator))
		);
		return CSubRegistrator(name);
	}

}