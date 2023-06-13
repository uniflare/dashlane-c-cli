#pragma once

#include <Types/transactions.h>
#include <pugixml.hpp>

namespace Utility
{

	namespace detail
	{
		inline void MapTransactionKeyValuePairs(const pugi::xml_node& node, std::unordered_map<std::string, std::string>& mappedPairs)
		{
			for (const auto& child : node.children())
			{
				mappedPairs[child.attribute("key").value()] = child.child_value();
			}
		}
	}

	inline std::unique_ptr<Dashlane::ITransactionBase> XmlToTransaction(const std::vector<uint8_t>& xmlBuffer)
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_buffer(xmlBuffer.data(), xmlBuffer.size());

		const pugi::xml_node& root = doc.child("root");

		if (root)
		{
			std::unordered_map<std::string, std::string> mappedPairs;
			if (const pugi::xml_node& authentifiant = root.child("KWAuthentifiant"))
			{
				detail::MapTransactionKeyValuePairs(authentifiant, mappedPairs);
				const nlohmann::ordered_json json(mappedPairs);
				return std::make_unique<Dashlane::STransactionAuthentifiant>(json.get<Dashlane::STransactionAuthentifiant>());
			}
			else if (const pugi::xml_node& authentifiant = root.child("KWSecureNote"))
			{
				detail::MapTransactionKeyValuePairs(authentifiant, mappedPairs);
				const nlohmann::ordered_json json(mappedPairs);
				return std::make_unique<Dashlane::STransactionSecureNote>(json.get<Dashlane::STransactionSecureNote>());
			}
		}

		return std::make_unique<Dashlane::STransactionUnknown>();
	}

	inline nlohmann::ordered_json XmlToJsonTransaction(const std::vector<uint8_t>& xmlBuffer)
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_buffer(xmlBuffer.data(), xmlBuffer.size());

		const pugi::xml_node& root = doc.child("root");
		if (root)
		{
			std::unordered_map<std::string, std::string> mappedPairs;
			if (const pugi::xml_node& authentifiant = root.child("KWAuthentifiant"))
			{
				detail::MapTransactionKeyValuePairs(authentifiant, mappedPairs);
				return nlohmann::ordered_json(mappedPairs);
			}
			else if (const pugi::xml_node& secureNote = root.child("KWSecureNote"))
			{
				detail::MapTransactionKeyValuePairs(secureNote, mappedPairs);
				return nlohmann::ordered_json(mappedPairs);
			}
		}

		return {};
	}

}