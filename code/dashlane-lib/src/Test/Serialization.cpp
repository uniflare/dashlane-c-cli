#include <dashlane/test/serialization.h>

#include "Encryption.h"
#include "Utility/Strings.h"

#include <iostream>

namespace Dashlane
{

	void expect(bool expected, const std::string& message)
	{
		if (!expected)
		{
			std::cerr << "TEST FAILED: " << message << std::endl;

			// Forced crash
			volatile int x = 0; x /= x;
		}
	}

	bool DoSerializationTest()
	{
		const std::string input = "The input string I want to encrypt";

		std::vector<uint8_t> key;
		key.resize(32);
		RAND_bytes(key.data(), key.size());

		DashlaneContextInternal ctx("", "");
		ctx.secrets.localKey = key;

		std::string encryptedInput;
		expect(EDashlaneError::NoError == EncryptAndSerialize(ctx, Utility::StringToVectorU8(input), encryptedInput), "Encrypt & Serialize failed");

		// decryption of encryption should successfully return the input
		{
			std::vector<uint8_t> decryptedInput;
			expect(EDashlaneError::NoError == DeserializeAndDecrypt(ctx, encryptedInput, decryptedInput), "Deserialize and Decrypt");
			const std::string decryptedString = Utility::VectorU8ToString(decryptedInput);
			expect(input == decryptedString, "Decryption failed");
		}

		return true;
	}

}