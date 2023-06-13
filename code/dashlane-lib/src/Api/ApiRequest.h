#pragma once

#include <set>

namespace Dashlane
{

	struct DashlaneContextInternal;

	enum class ERequestMethod
	{
		Get,
		Post
	};

	struct SAPIResponse
	{
		bool success{ false };
		int responseCode{ 0 };
		std::vector<uint8_t> body;
	};

	struct SApiErrorResponse
	{
		struct SError
		{
			std::string type;
			std::string code;
			std::string message;
		};

		std::vector<SError> errors;

		friend inline void from_json(const nlohmann::ordered_json& j, SApiErrorResponse::SError& p)
		{
			j.at("type").get_to(p.type);
			j.at("code").get_to(p.code);
			j.at("message").get_to(p.message);
		}

		friend inline void from_json(const nlohmann::ordered_json& j, SApiErrorResponse& p)
		{
			j.at("errors").get_to(p.errors);
		}
	};

	class CAPIRequest
	{

	public:

		CAPIRequest(
			ERequestMethod method, 
			const std::string& host, 
			const std::string& path,
			const std::string& signatureAlgorithm = "DL1-HMAC-SHA256",
			const std::map<std::string, std::string>& headers = {},
			const std::set<std::string>& signableHeaders = {},
			const std::map<std::string, std::string>& queries = {},
			const std::vector<uint8_t>& payload = {});

		CAPIRequest(const CAPIRequest& other) = delete;
		CAPIRequest(CAPIRequest&& other) = delete;

		~CAPIRequest();

		SAPIResponse SubmitRequest(const DashlaneContextInternal& context);

		bool AddHeader(const std::string& key, const std::string& value, bool signable = true);
		bool AddQuery(const std::string& key, const std::string& value);
		bool AddSignableHeader(const std::string& headerKey);

		void ClearHeaders();
		void ClearQueries();
		void ClearAll();

		void SetMethod(ERequestMethod method);
		void SetPath(const std::string& path);
		void SetPayload(const std::vector<uint8_t>& payload);
		void SetSignatureAlgorithm(const std::string& algorithm);

	protected:

		void InitializeCurl();

		std::string   GetAuthenticationHeaderString(const DashlaneContextInternal& context) const;
		std::string   GetAuthorizationHeader(const DashlaneContextInternal& context) const;
		std::string   GetCanonicalRequest() const;
		std::string   GetHashedPayload() const;
		std::string   GetHeadersString() const;
		std::string   GetRequestSignature(const DashlaneContextInternal& context, const std::string& timestamp) const;
		std::string   GetSecretKeyString(const DashlaneContextInternal& context) const;
		std::string   GetSignedHeadersString() const;
		std::string   GetURIEncodedPathString() const;
		std::string   GetURIEncodedQueryString() const;
		static size_t HandleResponseData(void* pContent, size_t unused, size_t contentSize, void* pUserData);
		std::string   MakeQueryStringFromPair(const std::pair<std::string, std::string> pair) const;
		std::string   URIEncodeString(const std::string_view& component) const;

	private:

		std::string m_signatureAlgorithm;

		ERequestMethod m_method{ ERequestMethod::Post };
		std::string    m_host;
		std::string    m_path;
		std::map<std::string, std::string> m_query;
		std::map<std::string, std::string> m_headers;
		std::set<std::string> m_signableHeaders;
		std::vector<uint8_t> m_payload;
	};

	EDashlaneError RequestApi(
		const DashlaneContextInternal& context, 
		const std::string& path, 
		nlohmann::ordered_json& output, 
		const nlohmann::ordered_json& payload);

}