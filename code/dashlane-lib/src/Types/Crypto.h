#pragma once

#include <Serialization.h>

namespace Dashlane
{

	enum class ECipherMode
	{
		CBCHMAC,
		CBCHMAC64
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(ECipherMode,
	{
		{ECipherMode::CBCHMAC, "cbchmac"},
		{ECipherMode::CBCHMAC64, "cbchmac64"}
	});

	inline bool Serialize(CSerializer& ser, ECipherMode& cipherMode)
	{
		// Serialize to string
		if (ser.IsOutput())
		{
			const std::string str = nlohmann::ordered_json(cipherMode);
			ser(str);
		}

		// Serialize from string
		else if (ser.IsInput())
		{
			std::string str;
			ser(str);
			cipherMode = nlohmann::ordered_json(str).get<ECipherMode>();
		}

		return true;
	}

	enum class EDerivationAlgorithm
	{
		None,
		Argon2D,
		PBKDF2
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(EDerivationAlgorithm,
	{
		{EDerivationAlgorithm::None, "noderivation"},
		{EDerivationAlgorithm::Argon2D, "argon2d"},
		{EDerivationAlgorithm::PBKDF2, "pbkdf2"}
	});

	struct SSymmetricCipherConfig
	{
		std::string encryption = "aes256";
		ECipherMode cipherMode{ ECipherMode::CBCHMAC };
		uint32_t ivLength{ 0 };

		bool Serialize(CSerializer& ser)
		{
			ser(encryption);
			ser(cipherMode);
			ser(ivLength);

			return true;
		}
	};

	struct SCipherData
	{
		std::vector<uint8_t> salt;
		std::vector<uint8_t> iv;
		std::vector<uint8_t> hash;
		std::vector<uint8_t> encryptedPayload;

		bool Serialize(CSerializer& ser)
		{
			// CipherData is packed without delimiters
			ser.SetSkipSeparators(true);

			if (ser.IsOutput())
			{
				// Salt might be empty
				ser(salt);
				ser(iv);
				ser(hash);
				ser(encryptedPayload);
			}
			// NOTE: Vectors need to have correct size.
			// A Size of 0 will eat remainder of buffer when skip separators is set to true.
			else // ser.IsInput()
			{
				if (salt.size() > 0)
					ser(salt);

				ser(iv);
				ser(hash);
				ser(encryptedPayload);
			}

			return true;
		}
	};

	struct IDerivationConfig
	{
		virtual EDerivationAlgorithm GetDerivation() const = 0;
		virtual const std::string GetDerivationName() const
		{
			return nlohmann::ordered_json(GetDerivation());
		}

		virtual bool HasSalt() const = 0;
		virtual size_t GetSaltLength() const = 0;

		virtual bool Serialize(CSerializer& ser) = 0;
	};

	struct SDerivationConfigNone : public IDerivationConfig
	{
		// IDerivationConfig
		virtual EDerivationAlgorithm GetDerivation() const override
		{
			return EDerivationAlgorithm::None;
		}

		virtual bool HasSalt() const override { return false; };
		virtual size_t GetSaltLength() const { return 0; };

		virtual bool Serialize(CSerializer& ser) override
		{
			if (ser.IsOutput())
			{
				std::string name = GetDerivationName();
				ser(name);
			}

			return true;
		}
		// ~IDerivationConfig
	};

	struct SDerivationConfigArgon2 : public IDerivationConfig
	{
		// IDerivationConfig
		virtual EDerivationAlgorithm GetDerivation() const override
		{
			return EDerivationAlgorithm::Argon2D;
		}

		virtual bool HasSalt() const override { return true; };
		virtual size_t GetSaltLength() const { return saltLength; };

		virtual bool Serialize(CSerializer& ser) override
		{
			if (ser.IsOutput())
			{
				std::string name = GetDerivationName();
				ser(name);
			}

			ser(saltLength);
			ser(tCost);
			ser(mCost);
			ser(parallelism);

			return true;
		}
		// ~IDerivationConfig

		uint32_t saltLength{ 0 };
		uint32_t tCost{ 0 };
		uint32_t mCost{ 0 };
		uint32_t parallelism{ 0 };
	};

	struct SDerivationConfigPbkdf2 : public IDerivationConfig
	{
		// IDerivationConfig
		virtual EDerivationAlgorithm GetDerivation() const override
		{
			return EDerivationAlgorithm::PBKDF2;
		}

		virtual bool HasSalt() const override { return true; };
		virtual size_t GetSaltLength() const { return saltLength; };

		virtual bool Serialize(CSerializer& ser) override
		{
			if (ser.IsOutput())
			{
				std::string name = GetDerivationName();
				ser(name);
			}

			ser(saltLength);
			ser(iterations);
			ser(hashMethod);

			return true;
		}
		// ~IDerivationConfig

		uint32_t saltLength{ 0 };
		uint32_t iterations{ 0 };
		std::string hashMethod;
	};

	struct SEncryptedData
	{
		static constexpr uint32_t expectedVersion = 1;
		uint32_t version = expectedVersion;
		std::unique_ptr<IDerivationConfig> pKeyDerivation;
		SSymmetricCipherConfig cipherConfig;
		SCipherData cipherData;

		bool Serialize(CSerializer& ser)
		{
			std::vector<uint8_t> empty;
			ser(empty);

			ser(version);

			if (ser.IsOutput())
			{
				ser(*pKeyDerivation);
			}
			else // IsInput()
			{
				std::string algorithm;
				ser(algorithm);
				switch (nlohmann::ordered_json(algorithm).get<EDerivationAlgorithm>())
				{
				case EDerivationAlgorithm::Argon2D:
				{
					pKeyDerivation = std::make_unique<SDerivationConfigArgon2>();
				} break;
				case EDerivationAlgorithm::PBKDF2:
				{
					pKeyDerivation = std::make_unique<SDerivationConfigPbkdf2>();
				} break;
				default:
					pKeyDerivation = std::make_unique<SDerivationConfigNone>();
					break;
				}
				ser(*pKeyDerivation);
			}

			ser(cipherConfig);

			// NOTE: We resize here since CipherData is packed without delimiters.
			// This means if the vector has a size of 0, it will eat the remainder of the buffer.
			// We specify the expected sizes here, using data from the CipherConfig and KeyDerivation.
			if (ser.IsInput())
			{
				const EDerivationAlgorithm algorithm = pKeyDerivation->GetDerivation();
				if (pKeyDerivation->HasSalt())
				{
					cipherData.salt.resize(pKeyDerivation->GetSaltLength());
				}
				cipherData.iv.resize(cipherConfig.ivLength);
				cipherData.hash.resize(32);
			}

			ser(cipherData);

			return true;
		}
	};

}