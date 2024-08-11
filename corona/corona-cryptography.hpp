/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This implements various cryptography wrappers around Windows crypto.

Notes

For Future Consideration
don't forget to encrypt key fields and traffic.

*/


#ifndef CORONA_CRYPTOGRAPHY
#define CORONA_CRYPTOGRAPHY

namespace corona
{
	class crypto
	{
		buffer get_crypto_buffer(BCRYPT_HANDLE _item, const wchar_t* _property_name)
		{
			DWORD bytes_length = {};
			DWORD result_bytes = {};
			NTSTATUS status;
			buffer new_buffer;

			if (wcscmp(_property_name, BCRYPT_AUTH_TAG_LENGTH) == 0)
			{
				BCRYPT_AUTH_TAG_LENGTHS_STRUCT atls = {};
				status = ::BCryptGetProperty(_item, _property_name, (PUCHAR)&atls, sizeof(atls), &result_bytes, 0);
				bytes_length = atls.dwMaxLength;
			}
			else
			{
				status = ::BCryptGetProperty(_item, _property_name, (PUCHAR)&bytes_length, sizeof(bytes_length), &result_bytes, 0);
			}

			if (!status) {
				new_buffer = buffer(bytes_length);
			}
			else
			{
				throw std::logic_error("Cannot get crypto property");
			}
			return new_buffer;
		}

	public:

		std::string hash(std::string _src)
		{
			std::string signature;

			BCRYPT_ALG_HANDLE algorithm = {};
			BCRYPT_HASH_HANDLE hash_handle = {};

			NTSTATUS status;

			status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);

			if (algorithm)
			{
				status = BCryptCreateHash(algorithm, &hash_handle, nullptr, 0, nullptr, 0, 0);

				if (hash_handle)
				{
					std::string hash_data = _src;
					status = BCryptHashData(hash_handle, (PUCHAR)hash_data.c_str(), hash_data.size(), 0);
					if (!status)
					{
						DWORD hash_length = {};
						DWORD count_bytes = {};
						status = ::BCryptGetProperty(hash_handle, BCRYPT_HASH_LENGTH, (PUCHAR)&hash_length, sizeof(hash_length), &count_bytes, 0);

						if (hash_length)
						{
							buffer buff(hash_length);
							::BCryptFinishHash(hash_handle, (PUCHAR)buff.get_ptr(), hash_length, 0);

							signature = buff.to_hex();
						}
					}
				}
			}

			if (hash_handle)
			{
				BCryptDestroyHash(hash_handle);
			}

			if (algorithm)
			{
				BCryptCloseAlgorithmProvider(algorithm, 0);
			}

			return signature;
		}

		std::string hash(json& _src)
		{
			std::string signature;

			BCRYPT_ALG_HANDLE algorithm = {};
			BCRYPT_HASH_HANDLE hash_handle = {};

			NTSTATUS status;

			status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);

			if (algorithm)
			{
				status = BCryptCreateHash(algorithm, &hash_handle, nullptr, 0, nullptr, 0, 0);

				if (hash_handle)
				{
					std::string hash_data = _src.to_json_typed();
					status = BCryptHashData(hash_handle, (PUCHAR)hash_data.c_str(), hash_data.size(), 0);
					if (!status)
					{
						DWORD hash_length = {};
						DWORD count_bytes = {};
						status = ::BCryptGetProperty(hash_handle, BCRYPT_HASH_LENGTH, (PUCHAR)&hash_length, sizeof(hash_length), &count_bytes, 0);

						if (hash_length)
						{
							buffer buff(hash_length);
							::BCryptFinishHash(hash_handle, (PUCHAR)buff.get_ptr(), hash_length, 0);

							signature = buff.to_hex();
						}
					}
				}
			}

			if (hash_handle)
			{
				BCryptDestroyHash(hash_handle);
			}

			if (algorithm)
			{
				BCryptCloseAlgorithmProvider(algorithm, 0);
			}

			return signature;
		}

		std::string encrypt(json& _src, std::string _pass_phrase, std::string _iv)
		{
			std::string cipher_text;
			std::string plain_text;

			plain_text = _src.to_json_typed();

			BCRYPT_ALG_HANDLE algorithm = {};
			BCRYPT_KEY_HANDLE key = {};

			NTSTATUS status;

			status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_AES_ALGORITHM, 0, 0);

			if (algorithm)
			{
				buffer key_buffer = get_crypto_buffer(algorithm, BCRYPT_OBJECT_LENGTH);
				buffer iv_buffer = get_crypto_buffer(algorithm, BCRYPT_BLOCK_LENGTH);
				buffer pass_buffer = get_crypto_buffer(algorithm, BCRYPT_BLOCK_LENGTH);

				PUCHAR pkey = key_buffer.get_uptr();
				DWORD skey = key_buffer.get_size();
				PUCHAR ppass = pass_buffer.get_uptr();
				DWORD spass = pass_buffer.get_size();
				pass_buffer.set_buffer(_pass_phrase);

				// Generate the key from supplied input key bytes.
				if (status = BCryptGenerateSymmetricKey(
					algorithm,
					&key,
					key_buffer.get_uptr(),
					key_buffer.get_size(),
					pass_buffer.get_uptr(),
					pass_buffer.get_size(),
					0))
				{
					os_result osr;
					std::cout << osr << std::endl;
					goto cleanup;
				}

				if (key)
				{
					DWORD cipher_text_size = {};

					if (status = BCryptEncrypt(
						key,
						(PUCHAR)plain_text.c_str(),
						plain_text.size(),
						nullptr,
						iv_buffer.get_uptr(),
						iv_buffer.get_size(),
						NULL,
						0,
						&cipher_text_size,
						BCRYPT_BLOCK_PADDING))
					{
						os_result osr;
						std::cout << osr << std::endl;
						goto cleanup;
					}

					/*

					iv_buffer.get_uptr(),
											iv_buffer.get_size(),
					*/

					buffer cipher_text_buffer(cipher_text_size);

					if (status = BCryptEncrypt(
						key,
						(PUCHAR)plain_text.c_str(),
						plain_text.size(),
						nullptr,
						nullptr,
						0,
						cipher_text_buffer.get_uptr(),
						cipher_text_buffer.get_size(),
						&cipher_text_size,
						BCRYPT_BLOCK_PADDING))
					{
						os_result osr;
						std::cout << osr << std::endl;
						goto cleanup;
					}

					cipher_text = cipher_text_buffer.to_hex(cipher_text_size);
				}
			}


		cleanup:

			if (key)
			{
				BCryptDestroyKey(key);
			}

			if (algorithm)
			{
				BCryptCloseAlgorithmProvider(algorithm, 0);
			}

			return cipher_text;
		}

		std::string decrypt(std::string cipher_text, std::string _pass_phrase, std::string _iv)
		{
			std::string plain_text;

			BCRYPT_ALG_HANDLE algorithm = {};
			BCRYPT_KEY_HANDLE key = {};

			NTSTATUS status;

			status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_AES_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
			int cipher_text_length = cipher_text.size();
			buffer cipher_buffer(cipher_text_length);
			cipher_buffer.from_hex(cipher_text);

			if (algorithm)
			{
				buffer key_buffer = get_crypto_buffer(algorithm, BCRYPT_OBJECT_LENGTH);
				buffer iv_buffer = get_crypto_buffer(algorithm, BCRYPT_BLOCK_LENGTH);
				buffer pass_buffer = get_crypto_buffer(algorithm, BCRYPT_BLOCK_LENGTH);

				pass_buffer.set_buffer(_pass_phrase);
				iv_buffer.set_buffer(_iv);

				// Generate the key from supplied input key bytes.
				if (status = BCryptGenerateSymmetricKey(
					algorithm,
					&key,
					key_buffer.get_uptr(),
					key_buffer.get_size(),
					pass_buffer.get_uptr(),
					pass_buffer.get_size(),
					0))
				{
					goto cleanup;
				}

				if (key)
				{
					DWORD plain_text_size = {};

					if (status = BCryptDecrypt(
						key,
						cipher_buffer.get_uptr(),
						cipher_buffer.get_size(),
						NULL,
						nullptr,
						0,
						NULL,
						0,
						&plain_text_size,
						BCRYPT_BLOCK_PADDING))
					{
						goto cleanup;
					}

					buffer plain_text_buffer(plain_text_size);

					if (status = BCryptDecrypt(
						key,
						cipher_buffer.get_uptr(),
						cipher_buffer.get_size(),
						NULL,
						nullptr,
						0,
						plain_text_buffer.get_uptr(),
						plain_text_buffer.get_size(),
						&plain_text_size,
						BCRYPT_BLOCK_PADDING))
					{
						goto cleanup;
					}

					plain_text = plain_text_buffer.get_ptr();
				}
			}

		cleanup:

			if (key)
			{
				BCryptDestroyKey(key);
			}

			if (algorithm)
			{
				BCryptCloseAlgorithmProvider(algorithm, 0);
			}

			return plain_text;
		}
	};

	class certificates {
	public:

		bool InstallRootCertificate(const std::string& certPath) {
			// Open the certificate file
			HANDLE hFile = CreateFile(certPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) {
				std::cerr << "Failed to open certificate file." << std::endl;
				return false;
			}

			// Get the file size
			DWORD fileSize = GetFileSize(hFile, NULL);
			if (fileSize == INVALID_FILE_SIZE) {
				std::cerr << "Failed to get certificate file size." << std::endl;
				CloseHandle(hFile);
				return false;
			}

			// Read the certificate file into memory
			BYTE* certData = new BYTE[fileSize];
			DWORD bytesRead;
			if (!ReadFile(hFile, certData, fileSize, &bytesRead, NULL)) {
				std::cerr << "Failed to read certificate file." << std::endl;
				delete[] certData;
				CloseHandle(hFile);
				return false;
			}
			CloseHandle(hFile);

			// Create a certificate context from the file data
			PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, certData, fileSize);
			delete[] certData;
			if (!pCertContext) {
				std::cerr << "Failed to create certificate context." << std::endl;
				return false;
			}

			// Open the root certificate store
			HCERTSTORE hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"ROOT");
			if (!hStore) {
				std::cerr << "Failed to open root certificate store." << std::endl;
				CertFreeCertificateContext(pCertContext);
				return false;
			}

			// Add the certificate to the store
			if (!CertAddCertificateContextToStore(hStore, pCertContext, CERT_STORE_ADD_REPLACE_EXISTING, NULL)) {
				std::cerr << "Failed to add certificate to store." << std::endl;
				CertCloseStore(hStore, 0);
				CertFreeCertificateContext(pCertContext);
				return false;
			}

			// Clean up
			CertCloseStore(hStore, 0);
			CertFreeCertificateContext(pCertContext);

			std::cout << "Certificate installed successfully." << std::endl;
			return true;
		}
	};

	bool test_encryption()
	{
		crypto crypter;

		std::string hash_src1 = "a test of life";
		std::string hash_src2 = "a test of life";
		std::string hash_src3 = "a test of life 42";

		std::string hash_check1 = crypter.hash(hash_src1);
		std::string hash_check2 = crypter.hash(hash_src2);
		std::string hash_check3 = crypter.hash(hash_src3);

		if (hash_check1 != hash_check2)
		{
			std::cout << __FILE__ << " " << __LINE__ << ": Hash check failed, not equal for same hashes." << std::endl;
			return false;
		}
		if (hash_check1 == hash_check3)
		{
			std::cout << __FILE__ << " " << __LINE__ << ": Hash check failed, equal for different hashes." << std::endl;
			return false;
		}
		if (hash_check2 == hash_check3)
		{
			std::cout << __FILE__ << " " << __LINE__ << ": Hash check failed, equal for different hashes." << std::endl;
			return false;
		}

		json_parser jp;
		json test1 = jp.create_object("Test", hash_src1);
		json test2 = jp.create_object("Test", hash_src2);
		json test3 = jp.create_object("Test", hash_src3);
		std::string test_encrypt1 = crypter.encrypt(test1, "test key", "2134234");
		std::string test_encrypt2 = crypter.encrypt(test2, "test key", "2134234");
		std::string test_encrypt3 = crypter.encrypt(test3, "test key", "2134234");
		std::string test_encrypt4 = crypter.encrypt(test3, "test other key", "2134234");

		if (test_encrypt1 != test_encrypt2)
		{
			std::cout << __FILE__ << " " << __LINE__ << ": encrypt check failed, not equal for same key / plaintext." << std::endl;
			return false;
		}
		if (test_encrypt1 == test_encrypt3)
		{
			std::cout << __FILE__ << " " << __LINE__ << ": encrypt check failed, equal for different hashes." << std::endl;
			return false;
		}
		if (test_encrypt3 == test_encrypt4)
		{
			std::cout << __FILE__ << " " << __LINE__ << ": encrypt check failed, changing key had insufficient impact." << std::endl;
			return false;
		}

	}
}

#endif
