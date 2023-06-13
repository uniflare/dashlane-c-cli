#pragma once

namespace Utility
{

	static constexpr int32_t OPENSSL_RC_SUCCESS = 1;
	static constexpr size_t SHA256_DIGEST_SIZE = 32;
	static constexpr size_t SHA512_DIGEST_SIZE = 64;

	template <typename T1, typename T2>
	inline std::vector<uint8_t> HmacSHA256(const T1& key, const T2& data)
	{
		std::vector<uint8_t> result;
		result.resize(SHA256_DIGEST_SIZE);

		auto hmac_result = reinterpret_cast<unsigned char*>(result.data());
		auto hmac_data = reinterpret_cast<const unsigned char*>(data.data());

		HMAC(EVP_sha256(), key.data(), key.size(), hmac_data, data.size(), hmac_result, nullptr);

		return result;
	}

	template<typename T>
	inline std::vector<uint8_t> SHA256(const T& input)
	{
		std::vector<uint8_t> result;

		if (!input.empty())
		{
			if (EVP_MD_CTX* pCtx = EVP_MD_CTX_new())
			{
				if (EVP_DigestInit_ex(pCtx, EVP_sha256(), NULL) == OPENSSL_RC_SUCCESS)
				{
					if (EVP_DigestUpdate(pCtx, input.data(), input.size()) == OPENSSL_RC_SUCCESS)
					{
						result.resize(SHA256_DIGEST_LENGTH);
						unsigned int length = 0;
						EVP_DigestFinal_ex(pCtx, result.data(), &length);
					}
				}

				EVP_MD_CTX_free(pCtx);
			}
		}

		return result;
	}

	template <typename T>
	inline std::vector<uint8_t> SHA512(const T& input)
	{
		std::vector<uint8_t> result;

		if (!input.empty())
		{
			if (EVP_MD_CTX* pCtx = EVP_MD_CTX_new())
			{
				if (EVP_DigestInit_ex(pCtx, EVP_sha512(), NULL) == OPENSSL_RC_SUCCESS)
				{
					if (EVP_DigestUpdate(pCtx, input.data(), input.size()) == OPENSSL_RC_SUCCESS)
					{
						result.resize(SHA512_DIGEST_LENGTH);
						unsigned int length = 0;
						EVP_DigestFinal_ex(pCtx, result.data(), &length);
					}
				}

				EVP_MD_CTX_free(pCtx);
			}
		}

		return result;
	}

	inline std::vector<uint8_t> GenerateRandomData(size_t size)
	{
		std::vector<uint8_t> data;
		data.resize(size);
		RAND_bytes(data.data(), size);
		return data;
	}

}