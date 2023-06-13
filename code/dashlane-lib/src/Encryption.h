#pragma once

#include "Dashlane.h"
#include "Types/Crypto.h"

namespace Dashlane
{

	struct SEncryptionContext
	{
		bool useKeyRegistry{ false };

		std::vector<uint8_t> symmetricKey{};
		std::vector<uint8_t> cipherKey{};
		std::vector<uint8_t> hmacKey{};

		ECipherMode cipherMode{ ECipherMode::CBCHMAC };
		std::unique_ptr<IDerivationConfig> pKeyDerivationConfig{};
		std::vector<uint8_t> salt{};
		std::vector<uint8_t> iv{};
		std::vector<uint8_t> hash{};

		std::vector<uint8_t> input{};
		std::vector<uint8_t> output{};
	};

	class CEncryption
	{

	public:

		const std::vector<uint8_t>& GetOutput() const { return m_context.output; }

		bool EncryptData(
			const std::vector<uint8_t>& symmetricKey, 
			const std::vector<uint8_t>& input, 
			SEncryptedData& encryptedDataOut);

		bool DecryptFromContext();

		bool SetContextFromEncryptedData(
			const std::vector<uint8_t>& symmetricKey, 
			const std::vector<uint8_t>& input, 
			SEncryptedData& data);

		void ResetContext();

		EDashlaneError GetSymmetricKeyFromData(
			const DashlaneContextInternal& context, 
			const std::vector<uint8_t>& rawPayload, 
			const SEncryptedData& encryptedData, 
			std::vector<uint8_t>& symmetricKey) const;

		bool GetSymmetricKeyViaDerivate(
			const IDerivationConfig& config, 
			const std::vector<uint8_t>& salt, 
			const std::string masterPassword, 
			std::vector<uint8_t>& symmetricKey) const;

	protected:

		bool GenerateRandomIV();
		std::vector<uint8_t> CreateSignatureHash() const;

		// Use output parameters instead of return tuple
		std::tuple<std::vector<uint8_t>, std::vector<uint8_t>> SplitSymmetricKey(const std::vector<uint8_t>& symmetricKey) const;

		bool EncryptWithAES256();
		bool DecryptWithAES256();

	private:

		SEncryptionContext m_context;

	};

}