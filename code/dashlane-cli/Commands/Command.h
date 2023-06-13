#pragma once

#include <CommandRegistry.h>

namespace Dashlane
{

	class CCommand
	{

	public:

		CCommand(const std::string& name, std::string&& desc, CLICommand&& cmd, CommandRegistrator additionalRegistrator = nullptr, std::string&& parentName = "")
			: m_name(name)
			, m_desc(std::move(desc))
			, m_cmd(std::move(cmd))
			, m_additionalRegistrator(std::move(additionalRegistrator))
			, m_parentName(std::move(parentName))
		{}

		const std::string& GetName() const
		{
			return m_name;
		}

		const std::string& GetDesc() const
		{
			return m_name;
		}

		const std::string& GetParentName() const
		{
			return m_parentName;
		}

		const CommandRegistrator& GetAdditionalRegistrator() const
		{
			return m_additionalRegistrator;
		}

		CLI::App* operator()(CLI::App* pApp) const
		{
			return m_cmd(pApp);
		};

	private:
		const std::string m_parentName;

		const std::string m_name;
		const std::string m_desc;
		const CLICommand m_cmd;
		const CommandRegistrator m_additionalRegistrator;

	};

}