#include "ApiRequest.h"

#include <Dashlane.h>
#include <Serialization.h>
#include <Utility/Cryptography.h>
#include <Utility/Filesystem.h>
#include <Utility/Strings.h>
#include <Utility/Time.h>

#include <curl/curl.h>

namespace Dashlane
{

	static CURL* s_pCurl;

#ifdef _DEBUG
	int DebugCurlCallback(CURL* pCurl, curl_infotype type, char* data, size_t size, void* userptr)
	{
		static const std::string prefix = "[DEBUG] cURL: ";
		static curl_infotype lastType = type;

		if (type == CURLINFO_TEXT || type == CURLINFO_HEADER_IN || type == CURLINFO_HEADER_OUT || type == CURLINFO_DATA_IN || type == CURLINFO_DATA_OUT)
		{
			std::string output;
			size_t prefixSize = 0;

			if (!(lastType == CURLINFO_DATA_IN && type == CURLINFO_DATA_IN) && !(lastType == CURLINFO_DATA_OUT && type == CURLINFO_DATA_OUT))
			{
				output = prefix;
				prefixSize = prefix.size();
			}

			output.resize(prefixSize + size);
			std::memcpy(output.data() + prefixSize, data, size);

#ifdef WINDOWS
			OutputDebugStringA(output.data());
#else
			std::cerr << "[DEBUG] cURL: " << safe << std::endl;
#endif
		}

		lastType = type;

		return CURLE_OK;
	}
#endif

	CAPIRequest::CAPIRequest(
		ERequestMethod method, 
		const std::string& host, 
		const std::string& path,
		const std::string& signatureAlgorithm,
		const std::map<std::string, std::string>& headers,
		const std::set<std::string>& signableHeaders,
		const std::map<std::string, std::string>& queries,
		const std::vector<uint8_t>& payload
	)
		: m_signatureAlgorithm(signatureAlgorithm)
		, m_method(method)
		, m_host(host)
		, m_path(path)
		, m_query(queries)
		, m_headers(headers)
		, m_signableHeaders(signableHeaders)
		, m_payload(payload)
	{
		InitializeCurl();
	}

	CAPIRequest::~CAPIRequest()
    {
        if (s_pCurl != nullptr)
        {
			curl_easy_cleanup(s_pCurl);
			s_pCurl = nullptr;
        }
	}

	SAPIResponse CAPIRequest::SubmitRequest(const DashlaneContextInternal& context)
	{
		SAPIResponse response{};

		// URI
		CURLU* pUrl = curl_url();
		std::string path = std::format("{}", m_path);
		curl_url_set(pUrl, CURLUPART_SCHEME, "https", 0);
		curl_url_set(pUrl, CURLUPART_HOST, m_host.c_str(), 0);
		curl_url_set(pUrl, CURLUPART_PATH, path.c_str(), 0);
        for (const auto& [key, value] : m_query)
		{
			const std::string query = std::format("{}={}", key, value);
			curl_url_set(pUrl, CURLUPART_QUERY, query.c_str(), CURLU_APPENDQUERY);
		}
		curl_easy_setopt(s_pCurl, CURLOPT_CURLU, pUrl);

		// Request Headers
		if (!m_headers.contains("user-agent"))
		{
			AddHeader("user-agent", "CI");
		}
		AddHeader("content-type", "application/json");
		AddHeader("host", m_host, false);

		curl_slist* pHeaders = nullptr;
		for (const auto& [key, value] : m_headers)
		{
            const std::string header = std::format("{}: {}", key, value);
			pHeaders = curl_slist_append(pHeaders, header.c_str());
		}

		const std::string authorizationHeader = GetAuthorizationHeader(context);
		pHeaders = curl_slist_append(pHeaders, authorizationHeader.c_str());
		curl_easy_setopt(s_pCurl, CURLOPT_HTTPHEADER, pHeaders);

		// Request Method & Data
		if (m_method == ERequestMethod::Post || m_payload.size() > 0)
		{
			curl_easy_setopt(s_pCurl, CURLOPT_POST, 1L);
			curl_easy_setopt(s_pCurl, CURLOPT_POSTFIELDS, m_payload.data());
			curl_easy_setopt(s_pCurl, CURLOPT_POSTFIELDSIZE, m_payload.size());
		}

		curl_easy_setopt(s_pCurl, CURLOPT_SSL_VERIFYPEER, 1);
		curl_easy_setopt(s_pCurl, CURLOPT_SSL_VERIFYHOST, 1);
#ifdef WINDOWS
		curl_easy_setopt(s_pCurl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif
		curl_easy_setopt(s_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(s_pCurl, CURLOPT_WRITEFUNCTION, HandleResponseData);
		curl_easy_setopt(s_pCurl, CURLOPT_WRITEDATA, (void*)&response.body);

#ifdef _DEBUG
		// Verbose Curl Output
		curl_easy_setopt(s_pCurl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(s_pCurl, CURLOPT_DEBUGDATA, nullptr);
		curl_easy_setopt(s_pCurl, CURLOPT_DEBUGFUNCTION, DebugCurlCallback);
#endif

		const CURLcode rc = curl_easy_perform(s_pCurl);
		response.success = rc == CURLE_OK;
		response.responseCode = rc;
		if (!response.success)
		{
            response.body = Utility::StringToVectorU8(curl_easy_strerror(rc));
		}
        else
        {
            curl_slist_free_all(pHeaders);
            curl_url_cleanup(pUrl);
        }

        return response;
    }

    bool CAPIRequest::AddHeader(const std::string& key, const std::string& value, bool signable)
	{
		std::string lowerKey;
		std::ranges::copy(key | std::ranges::views::transform([](auto c) { return std::tolower(c); }), std::back_inserter(lowerKey));

		if (!m_headers.contains(lowerKey))
		{
			m_headers.emplace(lowerKey, value);

			if (signable)
			{
				AddSignableHeader(lowerKey);
			}

			return true;
		}

        return false;
	}

	bool CAPIRequest::AddQuery(const std::string& key, const std::string& value)
	{
		if (!m_query.contains(key))
		{
            m_query.emplace(key, value);
			return true;
		}

		return false;
	}

	bool CAPIRequest::AddSignableHeader(const std::string& headerKey)
	{
		std::string lowerKey;
		std::ranges::copy(headerKey | std::ranges::views::transform([](auto c) { return std::tolower(c); }), std::back_inserter(lowerKey));
        return std::get<1>(m_signableHeaders.emplace(lowerKey));
	}

	void CAPIRequest::ClearHeaders()
	{
		m_headers.clear();
		m_signableHeaders.clear();
	}

	void CAPIRequest::ClearQueries()
	{
		m_query.clear();
	}

	void CAPIRequest::ClearAll()
	{
		ClearHeaders();
		ClearQueries();
		SetPayload({});
	}

	void CAPIRequest::SetMethod(ERequestMethod method)
	{
		m_method = method;
	}

	void CAPIRequest::SetPath(const std::string& path)
	{
		m_path = path;
	}

	void CAPIRequest::SetPayload(const std::vector<uint8_t>& payload)
    {
        m_payload = payload;
    }

	void CAPIRequest::SetSignatureAlgorithm(const std::string& algorithm)
	{
		m_signatureAlgorithm = algorithm;
	}

	void CAPIRequest::InitializeCurl()
	{
		if (s_pCurl == nullptr)
		{
			s_pCurl = curl_easy_init();
		}
	}

	std::string CAPIRequest::GetAuthenticationHeaderString(const DashlaneContextInternal& context) const
	{
		// Todo: Team device support
		std::string authenticationHeader;

		if (context.secrets.device.accessKey.empty())
		{
			authenticationHeader = std::format("{}={}", "AppAccessKey", context.secrets.app.accessKey);
		}
		else
		{
			authenticationHeader = Utility::StringJoin(",",
				std::format("{}={}", "Login", context.login),
				std::format("{}={}", "AppAccessKey", context.secrets.app.accessKey),
				std::format("{}={}", "DeviceAccessKey", context.secrets.device.accessKey)
			);
		}

		return authenticationHeader;
	}

	std::string CAPIRequest::GetAuthorizationHeader(const DashlaneContextInternal& context) const
	{
		const std::string timestamp = std::to_string(Utility::GetUnixTimestamp());
		const std::string authenticationHeader = GetAuthenticationHeaderString(context);
		const std::string signedHeaders = GetSignedHeadersString();
		const std::string signature = GetRequestSignature(context, timestamp);

		return std::format("Authorization: {} {}",
			m_signatureAlgorithm,
			Utility::StringJoin(",",
				authenticationHeader,
				std::format("{}={}", "Timestamp", timestamp),
				std::format("{}={}", "SignedHeaders", signedHeaders),
				std::format("{}={}", "Signature", signature)
			)
		);
	}

	std::string CAPIRequest::GetCanonicalRequest() const
	{
		std::string canonicalString = Utility::StringJoin("\n",
			m_method == ERequestMethod::Get ? "GET" : "POST",
			GetURIEncodedPathString(),
			GetURIEncodedQueryString(),
			GetHeadersString(),
			GetSignedHeadersString(),
			GetHashedPayload()
		);

		return canonicalString;
	}

	std::string CAPIRequest::GetHashedPayload() const
	{
		std::string hashedPayload;

		if (m_method == ERequestMethod::Post)
		{
			hashedPayload = Utility::ToHex(Utility::SHA256(m_payload));
		}

		return hashedPayload;
	}

	std::string CAPIRequest::GetHeadersString() const
	{
		std::string headers;

		for (const auto& key : m_signableHeaders)
		{
			std::string header = std::format("{}:{}\n", key, m_headers.at(key));
			headers.append(std::move(header));
		}

		return headers;
	}

	std::string CAPIRequest::GetRequestSignature(const DashlaneContextInternal& context, const std::string& timestamp) const
	{
		std::string secretKey = GetSecretKeyString(context);
		std::string canonicalRequest = GetCanonicalRequest();
		auto canonHash = Utility::SHA256(canonicalRequest);
		std::string canonHex = Utility::ToHex(canonHash);

		std::string stringToSign = Utility::StringJoin("\n",
			m_signatureAlgorithm,
			timestamp,
			canonHex
		);

		return Utility::ToHex(Utility::HmacSHA256(secretKey, stringToSign));
	}

	std::string CAPIRequest::GetSecretKeyString(const DashlaneContextInternal& context) const
	{
		std::string secretKey = context.secrets.app.secretKey;

		if (!context.secrets.device.secretKey.empty())
		{
			secretKey += "\n";
			secretKey += context.secrets.device.secretKey;
		}

		return secretKey;
	}

	std::string CAPIRequest::GetSignedHeadersString() const
	{
		std::string signedHeaders;

		auto it = m_signableHeaders.begin();
		if (it != m_signableHeaders.end())
		{
			signedHeaders = *it++;

			while (it != m_signableHeaders.end())
			{
				signedHeaders.push_back(';');
				signedHeaders.append(*it++);
			}
		}

		return signedHeaders;
	}

	std::string CAPIRequest::GetURIEncodedPathString() const
	{
		std::string encodedPath;
		encodedPath.reserve(m_path.size());

		for (const auto& view : std::views::split(m_path, '/'))
		{
			if (!view.empty())
			{
				encodedPath.push_back('/');
				encodedPath.append(URIEncodeString({ view.begin(), view.end() }));
			}
		}

		return encodedPath;
	}

	std::string CAPIRequest::GetURIEncodedQueryString() const
	{
		std::string encodedQuery;

		auto it = m_query.begin();
		if (it != m_query.end())
		{
			encodedQuery = MakeQueryStringFromPair(*it);

			while (++it != m_query.end())
			{
				encodedQuery.push_back('&');
				encodedQuery.append(std::move(MakeQueryStringFromPair(*it)));
			}
		}

		return encodedQuery;
	}

	size_t CAPIRequest::HandleResponseData(void* pContent, size_t unused, size_t contentSize, void* pUserData)
	{
		auto& buffer = *static_cast<std::vector<uint8_t>*>(pUserData);
		buffer.insert(buffer.end(), static_cast<char*>(pContent), static_cast<char*>(pContent) + contentSize);
		return contentSize;
	}

	std::string CAPIRequest::MakeQueryStringFromPair(const std::pair<std::string, std::string> pair) const
	{
		return std::format("{}={}", URIEncodeString(pair.first), URIEncodeString(pair.second));
	}

	std::string CAPIRequest::URIEncodeString(const std::string_view& component) const
	{
		std::string encoded;
		if (char* escaped = curl_easy_escape(s_pCurl, component.data(), component.size()))
		{
			encoded = escaped;
			curl_free(escaped);
		}
		return encoded;
	}

	EDashlaneError RequestApi(const DashlaneContextInternal& context, const std::string& path, nlohmann::ordered_json& output, const nlohmann::ordered_json& payload)
	{
		auto request = Dashlane::CAPIRequest(
			Dashlane::ERequestMethod::Post,
			"api.dashlane.com",
			std::format("/v1/{}", path)
		);

		request.AddHeader("user-agent", context.applicationName);

		const std::string plainPayload = payload.dump();
		request.SetPayload(std::vector<uint8_t>(plainPayload.begin(), plainPayload.end()));

		Dashlane::SAPIResponse response = request.SubmitRequest(context);

		if (!response.success)
			return EDashlaneError::UnkownRequestError;

		output = nlohmann::ordered_json::parse(response.body);
		if (output.contains("errors"))
		{
			const auto apiErrors = output.get<SApiErrorResponse>();

#ifdef _DEBUG
			for (const auto& error : apiErrors.errors)
			{
				std::cerr << "\tType: " << error.type << std::endl;
				std::cerr << "\tCode: " << error.code << std::endl;
				std::cerr << "\tMessage: " << error.message << std::endl << std::endl;
			}
#endif

			// We only care about the first error each run
			const auto& error = apiErrors.errors[0];
			if (error.type == "invalid_request_error")
			{
				if (error.code == "invalid_authentication")
					return EDashlaneError::APIError_Authentication;

				else if (error.code == "out_of_bounds_timestamp")
					return EDashlaneError::APIError_Timeout;

				else if (error.code == "unknown_userdevice_key")
					return EDashlaneError::APIError_DeviceKey;

				else if (error.code == "invalid_endpoint")
					return EDashlaneError::APIError_EndPoint;
			}
			else if (error.type == "business_error")
			{
				if (error.code == "verification_failed")
					return EDashlaneError::AuthenticationFailed;
			}

			return EDashlaneError::InvalidAPIRequest;
		}

		return EDashlaneError::NoError;
	}

}