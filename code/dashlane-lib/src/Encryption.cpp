#include "StdAfx.h"
#include "Dashlane.h"
#include "Encryption.h"
#include "Utility/Cryptography.h"
#include "Utility/Transaction.h"
#include "Utility/Vector.h"
#include "Utility/Zip.h"

#include <argon2.h>

namespace Dashlane
{

	class CSymmetricKeyRegistry
	{

	public:

		static void AddKey(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& key)
		{
			m_registry[signature] = key;
		}

		static const std::vector<uint8_t>& GetKey(const std::vector<uint8_t>& signature)
		{
			static const std::vector<uint8_t> empty;

			if (IsRegistered(signature))
			{
				return m_registry[signature];
			}

			return empty;
		}

		static bool IsRegistered(const std::vector<uint8_t>& signature)
		{
			return m_registry.contains(signature);
		}

	private:

		static std::map<std::vector<std::uint8_t>, std::vector<uint8_t>> m_registry;

	};

	std::map<std::vector<uint8_t>, std::vector<uint8_t>> CSymmetricKeyRegistry::m_registry = {};

	void CEncryption::ResetContext()
	{
		m_context = std::move(SEncryptionContext());
	}

	bool CEncryption::EncryptData(const std::vector<uint8_t>& symmetricKey, const std::vector<uint8_t>& input, SEncryptedData& encryptedDataOut)
	{
		bool success = false;

		if (symmetricKey.size() > 0 && input.size() > 0)
		{
			m_context.input = input;
			if (GenerateRandomIV())
			{
				auto [cipherKey, hmacKey] = SplitSymmetricKey(symmetricKey);
				m_context.symmetricKey = symmetricKey;
				m_context.cipherKey = cipherKey;
				m_context.hmacKey = hmacKey;

				if (EncryptWithAES256())
				{
					m_context.input = m_context.output; // Signature generation uses input, which requires encrypted payload

					encryptedDataOut.pKeyDerivation = std::make_unique<SDerivationConfigNone>();
					encryptedDataOut.cipherConfig.ivLength = m_context.iv.size();
					encryptedDataOut.cipherConfig.cipherMode = ECipherMode::CBCHMAC;
					encryptedDataOut.cipherData.salt = {};
					encryptedDataOut.cipherData.iv = m_context.iv;
					encryptedDataOut.cipherData.hash = CreateSignatureHash();
					encryptedDataOut.cipherData.encryptedPayload = m_context.input;

					success = true;
				}
			}
		}

		return success;
	}

	bool CEncryption::DecryptFromContext()
	{
		if (CreateSignatureHash() != m_context.hash)
		{
			return false;
		}

		if (!DecryptWithAES256())
		{
			return false;
		}

		return true;
	}

	EDashlaneError CEncryption::GetSymmetricKeyFromData(
		const DashlaneContextInternal& context, 
		const std::vector<uint8_t>& rawPayload, 
		const SEncryptedData& encryptedData,
		std::vector<uint8_t>& symmetricKey
	) const
	{
		std::vector<uint8_t> keyIdentifierBytes;
		const uint32_t ivLength = encryptedData.cipherConfig.ivLength;
		const uint32_t payloadSize = encryptedData.cipherData.encryptedPayload.size();
		const uint32_t keyIdentifierBytesEnd = rawPayload.size() - ivLength - 32 - payloadSize;
		keyIdentifierBytes.assign(rawPayload.begin(), rawPayload.begin() + keyIdentifierBytesEnd);

		// Essentially we salt the key bytes in case the master password turns out to be invalid
		// TODO: We can remove this if we clear the registry if the master password is invalid?
		Utility::AppendToVector(keyIdentifierBytes, context.secrets.masterPassword);

		if (!CSymmetricKeyRegistry::IsRegistered(keyIdentifierBytes))
		{
			if (!GetSymmetricKeyViaDerivate(*encryptedData.pKeyDerivation, encryptedData.cipherData.salt, context.secrets.masterPassword, symmetricKey))
			{
				return EDashlaneError::InvalidMasterPassword;
			}

			CSymmetricKeyRegistry::AddKey(keyIdentifierBytes, symmetricKey);
		}
		else
		{
			symmetricKey = CSymmetricKeyRegistry::GetKey(keyIdentifierBytes);
		}

		return EDashlaneError::NoError;
	}

	bool CEncryption::GetSymmetricKeyViaDerivate(
		const IDerivationConfig& config, 
		const std::vector<uint8_t>& salt,
		const std::string masterPassword,
		std::vector<uint8_t>& symmetricKey
	) const
	{
		symmetricKey.resize(32);

		switch (config.GetDerivation())
		{
		case EDerivationAlgorithm::Argon2D:
		{
			const auto& argon2 = static_cast<const SDerivationConfigArgon2&>(config);
			return ARGON2_OK == argon2d_hash_raw(
				argon2.tCost,
				argon2.mCost,
				argon2.parallelism,
				masterPassword.c_str(),
				masterPassword.size(),
				salt.data(),
				argon2.saltLength,
				symmetricKey.data(), 32);

		} break;

		case EDerivationAlgorithm::PBKDF2:
		{
			const auto& pbkdf2 = static_cast<const SDerivationConfigPbkdf2&>(config);
			return Utility::OPENSSL_RC_SUCCESS == PKCS5_PBKDF2_HMAC(
				masterPassword.c_str(),
				masterPassword.size(),
				salt.data(),
				pbkdf2.saltLength,
				pbkdf2.iterations,
				EVP_sha512(),
				32, symmetricKey.data());

		} break;
		}

		return true;
	}

	bool CEncryption::SetContextFromEncryptedData(const std::vector<uint8_t>& symmetricKey, const std::vector<uint8_t>& input, SEncryptedData& data)
	{
		if (symmetricKey.size() == 0)
		{
			return false;
		}

		if (input.size() == 0)
		{
			return false;
		}

		auto [cipherKey, hmacKey] = SplitSymmetricKey(symmetricKey);

		// Update context
		m_context.symmetricKey = symmetricKey;
		m_context.cipherKey = cipherKey;
		m_context.hmacKey = hmacKey;
		m_context.cipherMode = data.cipherConfig.cipherMode;
		m_context.pKeyDerivationConfig.swap(data.pKeyDerivation);
		m_context.salt = data.cipherData.salt;
		m_context.iv = data.cipherData.iv;
		m_context.hash = data.cipherData.hash;
		m_context.input = data.cipherData.encryptedPayload;

		return true;
	}

	bool CEncryption::GenerateRandomIV()
	{
		bool success = true;

		m_context.iv.resize(16);
		if (Utility::OPENSSL_RC_SUCCESS != RAND_bytes(m_context.iv.data(), m_context.iv.size()))
		{
			m_context.iv.resize(0);
			success = false;
		}

		return success;
	}

	std::vector<uint8_t> CEncryption::CreateSignatureHash() const
	{
		const std::vector<uint8_t> data = Utility::CreateChainedVector(m_context.iv, m_context.input);
		return Utility::HmacSHA256(m_context.hmacKey, data);
	}

	std::tuple<std::vector<uint8_t>, std::vector<uint8_t>> CEncryption::SplitSymmetricKey(const std::vector<uint8_t>& symmetricKey) const
	{
		std::vector<uint8_t> combinedKey = Utility::SHA512(symmetricKey);
		return { Utility::SliceVectorByCopy(combinedKey, 0, 32) ,Utility::SliceVectorByCopy(combinedKey, 32) };
	}

	bool CEncryption::EncryptWithAES256()
	{
		bool success = false;

		if (m_context.input.size() > 0)
		{
			if (EVP_CIPHER_CTX* pCtx = EVP_CIPHER_CTX_new())
			{
				const EVP_CIPHER* pCipher = EVP_aes_256_cbc();
				const size_t blockSize = EVP_CIPHER_block_size(pCipher);

				if (Utility::OPENSSL_RC_SUCCESS == EVP_EncryptInit_ex(pCtx, pCipher, NULL, m_context.cipherKey.data(), m_context.iv.data()))
				{
					m_context.output.resize(m_context.input.size() + blockSize - ((m_context.input.size() + blockSize) % blockSize));

					int written = 0;
					auto pInputData = reinterpret_cast<const unsigned char*>(m_context.input.data());
					auto pOutputData = reinterpret_cast<unsigned char*>(m_context.output.data());
					const auto inputSize = m_context.input.size();
					if (Utility::OPENSSL_RC_SUCCESS == EVP_EncryptUpdate(pCtx, pOutputData, &written, pInputData, inputSize))
					{
						if (Utility::OPENSSL_RC_SUCCESS == EVP_EncryptFinal_ex(pCtx, pOutputData + written, &written))
						{
							success = true;
						}
					}
				}

				EVP_CIPHER_CTX_free(pCtx);
			}
		}

		if (!success)
		{
			m_context.output.resize(0);
		}

		return success;
	}

	bool CEncryption::DecryptWithAES256()
	{
		bool success = false;

		if (EVP_CIPHER_CTX* pCtx = EVP_CIPHER_CTX_new())
		{
			int written = 0;
			int written_final = 0;

			if (Utility::OPENSSL_RC_SUCCESS == EVP_DecryptInit_ex(pCtx, EVP_aes_256_cbc(), NULL, m_context.cipherKey.data(), m_context.iv.data()))
			{
				m_context.output.resize(m_context.input.size());
				if (Utility::OPENSSL_RC_SUCCESS == EVP_DecryptUpdate(pCtx, m_context.output.data(), &written, m_context.input.data(), m_context.input.size()))
				{
					EVP_DecryptFinal_ex(pCtx, m_context.output.data() + written, &written_final);
					m_context.output.resize(written + written_final); // Shrink to fit
					success = true;
				}
			}

			EVP_CIPHER_CTX_free(pCtx);
		}

		if (!success)
		{
			m_context.output.resize(0);
		}

		return success;
	}

}