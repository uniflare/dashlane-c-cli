#pragma once

#include <CLI/CLI.hpp>

namespace Dashlane
{

	class CCommand;
	class CSubRegistrator;

	using CLICommand = std::function<CLI::App* (CLI::App*)>;
	using CommandRegistrator = std::function <void(CLI::App*)>;

	enum class EVisitorState
	{
		Continue,
		Stop
	};

	CSubRegistrator COMMAND(const std::string& name, std::string&& desc, CLICommand&& cmd, CommandRegistrator additionalRegistrator = nullptr);

	// TODO: Move registration into a hierarchical map
	class CSubRegistrator
	{

	public:

		CSubRegistrator(const std::string& parentName)
			: m_parentName(parentName)
		{};

		~CSubRegistrator()
		{};

		CSubRegistrator COMMAND(const std::string & name, std::string && desc, CLICommand && cmd, CommandRegistrator additionalRegistrator = nullptr)
		{
			std::string fullName = std::format("{}::{}", m_parentName, name);
			Dashlane::COMMAND(fullName, std::move(desc), std::move(cmd), additionalRegistrator);
			return *this;
		}

		CSubRegistrator SUBCOMMAND(const std::string& name, std::string&& desc, CLICommand&& cmd, CommandRegistrator additionalRegistrator = nullptr)
		{
			std::string fullName = std::format("{}::{}", m_parentName, name);
			return Dashlane::COMMAND(fullName, std::move(desc), std::move(cmd), additionalRegistrator);
		}

		const std::string m_parentName;
	};

	class CCommandRegistry
	{

		CCommandRegistry() = delete;
		~CCommandRegistry() = delete;

	public:

		static void Register(CCommand&& cmd);
		static void VisitCommands(std::function<EVisitorState(const CCommand&)> visitor);

	private:

		static std::vector<CCommand> s_registry;

	};

}