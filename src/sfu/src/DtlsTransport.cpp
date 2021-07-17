

#include "RTC/DtlsTransport.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Settings.h"
#include "Utils.h"
#include <openssl/asn1.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <uv.h>
#include <cstdio>  // std::sprintf(), std::fopen()
#include <cstring> // std::memcpy(), std::strcmp()

#define LOG_OPENSSL_ERROR(desc)                                                                    \
	do                                                                                               \
	{                                                                                                \
		if (ERR_peek_error() == 0)                                                                     \
			SError <<  "OpenSSL error [desc:] " <<  desc;                                                 \
		else                                                                                           \
		{                                                                                              \
			int64_t err;                                                                                 \
			while ((err = ERR_get_error()) != 0)                                                         \
			{                                                                                            \
				SError << "OpenSSL error [desc:] " << desc  <<  " [error:] " <<  ERR_error_string(err, nullptr);   \
			}                                                                                            \
			ERR_clear_error();                                                                           \
		}                                                                                              \
	} while (false)

/* Static methods for OpenSSL callbacks. */

inline static int onSslCertificateVerify(int /*preverifyOk*/, X509_STORE_CTX* /*ctx*/)
{
	

	// Always valid since DTLS certificates are self-signed.
	return 1;
}

inline static void onSslInfo(const SSL* ssl, int where, int ret)
{
	static_cast<RTC::DtlsTransport*>(SSL_get_ex_data(ssl, 0))->OnSslInfo(where, ret);
}

inline static unsigned int onSslDtlsTimer(SSL* /*ssl*/, unsigned int timerUs)
{
	if (timerUs == 0)
		return 100000;
	else if (timerUs >= 4000000)
		return 4000000;
	else
		return 2 * timerUs;
}

namespace RTC
{
	/* Static. */

	
	static constexpr int DtlsMtu{ 1350 };
	static constexpr int SslReadBufferSize{ 65536 };
	// AES-HMAC: http://tools.ietf.org/html/rfc3711
	static constexpr size_t SrtpMasterKeyLength{ 16 };
	static constexpr size_t SrtpMasterSaltLength{ 14 };
	static constexpr size_t SrtpMasterLength{ SrtpMasterKeyLength + SrtpMasterSaltLength };
	// AES-GCM: http://tools.ietf.org/html/rfc7714
	static constexpr size_t SrtpAesGcm256MasterKeyLength{ 32 };
	static constexpr size_t SrtpAesGcm256MasterSaltLength{ 12 };
	static constexpr size_t SrtpAesGcm256MasterLength{ SrtpAesGcm256MasterKeyLength + SrtpAesGcm256MasterSaltLength };
	static constexpr size_t SrtpAesGcm128MasterKeyLength{ 16 };
	static constexpr size_t SrtpAesGcm128MasterSaltLength{ 12 };
	static constexpr size_t SrtpAesGcm128MasterLength{ SrtpAesGcm128MasterKeyLength + SrtpAesGcm128MasterSaltLength };
	

	/* Class variables. */

	//X509* DtlsTransport::certificate{ nullptr };
	//EVP_PKEY* DtlsTransport::privateKey{ nullptr };
	SSL_CTX* DtlsTransport::sslCtx{ nullptr };
	uint8_t DtlsTransport::sslReadBuffer[SslReadBufferSize];
	
	std::map<std::string, DtlsTransport::FingerprintAlgorithm> DtlsTransport::string2FingerprintAlgorithm =
	{
		{ "sha-1",   DtlsTransport::FingerprintAlgorithm::SHA1   },
		{ "sha-224", DtlsTransport::FingerprintAlgorithm::SHA224 },
		{ "sha-256", DtlsTransport::FingerprintAlgorithm::SHA256 },
		{ "sha-384", DtlsTransport::FingerprintAlgorithm::SHA384 },
		{ "sha-512", DtlsTransport::FingerprintAlgorithm::SHA512 }
	};
	std::map<DtlsTransport::FingerprintAlgorithm, std::string> DtlsTransport::fingerprintAlgorithm2String =
	{
		{ DtlsTransport::FingerprintAlgorithm::SHA1,   "sha-1"   },
		{ DtlsTransport::FingerprintAlgorithm::SHA224, "sha-224" },
		{ DtlsTransport::FingerprintAlgorithm::SHA256, "sha-256" },
		{ DtlsTransport::FingerprintAlgorithm::SHA384, "sha-384" },
		{ DtlsTransport::FingerprintAlgorithm::SHA512, "sha-512" }
	};
	std::map<std::string, DtlsTransport::Role> DtlsTransport::string2Role =
	{
		{ "auto",   DtlsTransport::Role::AUTO   },
		{ "client", DtlsTransport::Role::CLIENT },
		{ "server", DtlsTransport::Role::SERVER }
	};
	
	std::vector<DtlsTransport::Fingerprint> DtlsTransport::localFingerprints;
	
	std::vector<DtlsTransport::SrtpProfileMapEntry> DtlsTransport::srtpProfiles =
	{
		{ RTC::SrtpSession::Profile::AEAD_AES_256_GCM, "SRTP_AEAD_AES_256_GCM" },
		{ RTC::SrtpSession::Profile::AEAD_AES_128_GCM, "SRTP_AEAD_AES_128_GCM" },
		{ RTC::SrtpSession::Profile::AES_CM_128_HMAC_SHA1_80, "SRTP_AES128_CM_SHA1_80" },
		{ RTC::SrtpSession::Profile::AES_CM_128_HMAC_SHA1_32, "SRTP_AES128_CM_SHA1_32" }
	};
	

	/* Class methods. */

	void DtlsTransport::ClassInit()
	{
            STrace << "DtlsTransport::ClassInit()";

		// Generate a X509 certificate and private key (unless PEM files are provided).
		if (
		  Settings::configuration.dtlsCertificateFile.empty() ||
		  Settings::configuration.dtlsPrivateKeyFile.empty())
		{
			//GenerateCertificateAndPrivateKey();
                    
                    SError << "No certificate files.";
                    base::uv::throwError("No certificate files");
                    //exit(0);
		}
//		else
//		{
//			ReadCertificateAndPrivateKeyFromFiles();
//		}

		// Create a global SSL_CTX.
		CreateSslCtx();

		// Generate certificate fingerprints.
		GenerateFingerprints();
	}

	void DtlsTransport::ClassDestroy()
	{
		

//		if (DtlsTransport::privateKey)
//			EVP_PKEY_free(DtlsTransport::privateKey);
//		if (DtlsTransport::certificate)
//			X509_free(DtlsTransport::certificate);
		if (DtlsTransport::sslCtx)
			SSL_CTX_free(DtlsTransport::sslCtx);
	}

//	void DtlsTransport::GenerateCertificateAndPrivateKey()
//	{
//		
//
//		int ret{ 0 };
//		BIGNUM* bne{ nullptr };
//		RSA* rsaKey{ nullptr };
//		int numBits{ 1024 };
//		X509_NAME* certName{ nullptr };
//		std::string subject =
//		  std::string("sfuserver") + std::to_string(Utils::Crypto::GetRandomUInt(100000, 999999));
//
//		// Create a big number object.
//		bne = BN_new();
//
//		if (!bne)
//		{
//			LOG_OPENSSL_ERROR("BN_new() failed");
//
//			goto error;
//		}
//
//		ret = BN_set_word(bne, RSA_F4); // RSA_F4 == 65537.
//
//		if (ret == 0)
//		{
//			LOG_OPENSSL_ERROR("BN_set_word() failed");
//
//			goto error;
//		}
//
//		// Generate a RSA key.
//		rsaKey = RSA_new();
//
//		if (!rsaKey)
//		{
//			LOG_OPENSSL_ERROR("RSA_new() failed");
//
//			goto error;
//		}
//
//		// This takes some time.
//		ret = RSA_generate_key_ex(rsaKey, numBits, bne, nullptr);
//
//		if (ret == 0)
//		{
//			LOG_OPENSSL_ERROR("RSA_generate_key_ex() failed");
//
//			goto error;
//		}
//
//		// Create a private key object (needed to hold the RSA key).
//		DtlsTransport::privateKey = EVP_PKEY_new();
//
//		if (!DtlsTransport::privateKey)
//		{
//			LOG_OPENSSL_ERROR("EVP_PKEY_new() failed");
//
//			goto error;
//		}
//
//		ret = EVP_PKEY_assign_RSA(DtlsTransport::privateKey, rsaKey); // NOLINT
//
//		if (ret == 0)
//		{
//			LOG_OPENSSL_ERROR("EVP_PKEY_assign_RSA() failed");
//
//			goto error;
//		}
//		// The RSA key now belongs to the private key, so don't clean it up separately.
//		rsaKey = nullptr;
//
//		// Create the X509 certificate.
//		DtlsTransport::certificate = X509_new();
//
//		if (!DtlsTransport::certificate)
//		{
//			LOG_OPENSSL_ERROR("X509_new() failed");
//
//			goto error;
//		}
//
//		// Set version 3 (note that 0 means version 1).
//		X509_set_version(DtlsTransport::certificate, 2);
//
//		// Set serial number (avoid default 0).
//		ASN1_INTEGER_set(
//		  X509_get_serialNumber(DtlsTransport::certificate),
//		  static_cast<uint64_t>(Utils::Crypto::GetRandomUInt(1000000, 9999999)));
//
//		// Set valid period.
//		X509_gmtime_adj(X509_get_notBefore(DtlsTransport::certificate), -315360000); // -10 years.
//		X509_gmtime_adj(X509_get_notAfter(DtlsTransport::certificate), 315360000);   // 10 years.
//
//		// Set the public key for the certificate using the key.
//		ret = X509_set_pubkey(DtlsTransport::certificate, DtlsTransport::privateKey);
//
//		if (ret == 0)
//		{
//			LOG_OPENSSL_ERROR("X509_set_pubkey() failed");
//
//			goto error;
//		}
//
//		// Set certificate fields.
//		certName = X509_get_subject_name(DtlsTransport::certificate);
//
//		if (!certName)
//		{
//			LOG_OPENSSL_ERROR("X509_get_subject_name() failed");
//
//			goto error;
//		}
//
//		X509_NAME_add_entry_by_txt(
//		  certName, "O", MBSTRING_ASC, reinterpret_cast<const uint8_t*>(subject.c_str()), -1, -1, 0);
//		X509_NAME_add_entry_by_txt(
//		  certName, "CN", MBSTRING_ASC, reinterpret_cast<const uint8_t*>(subject.c_str()), -1, -1, 0);
//
//		// It is self-signed so set the issuer name to be the same as the subject.
//		ret = X509_set_issuer_name(DtlsTransport::certificate, certName);
//
//		if (ret == 0)
//		{
//			LOG_OPENSSL_ERROR("X509_set_issuer_name() failed");
//
//			goto error;
//		}
//
//		// Sign the certificate with its own private key.
//		ret = X509_sign(DtlsTransport::certificate, DtlsTransport::privateKey, EVP_sha1());
//
//		if (ret == 0)
//		{
//			LOG_OPENSSL_ERROR("X509_sign() failed");
//
//			goto error;
//		}
//
//		// Free stuff and return.
//		BN_free(bne);
//
//		return;
//
//	error:
//
//		if (bne)
//			BN_free(bne);
//
//		if (rsaKey && !DtlsTransport::privateKey)
//			RSA_free(rsaKey);
//
//		if (DtlsTransport::privateKey)
//			EVP_PKEY_free(DtlsTransport::privateKey); // NOTE: This also frees the RSA key.
//
//		if (DtlsTransport::certificate)
//			X509_free(DtlsTransport::certificate);
//
//		base::uv::throwError("DTLS certificate and private key generation failed");
//	}

//	void DtlsTransport::ReadCertificateAndPrivateKeyFromFiles()
//	{
//		
//
//		FILE* file{ nullptr };
//
//		file = fopen(Settings::configuration.dtlsCertificateFile.c_str(), "r");
//
//		if (!file)
//		{
//			MS_ERROR("error reading DTLS certificate file: ", std::strerror(errno));
//
//			goto error;
//		}
//
//		DtlsTransport::certificate = PEM_read_X509(file, nullptr, nullptr, nullptr);
//
//		if (!DtlsTransport::certificate)
//		{
//			LOG_OPENSSL_ERROR("PEM_read_X509() failed");
//
//			goto error;
//		}
//
//		fclose(file);
//
//		file = fopen(Settings::configuration.dtlsPrivateKeyFile.c_str(), "r");
//
//		if (!file)
//		{
//			MS_ERROR("error reading DTLS private key file: ", std::strerror(errno));
//
//			goto error;
//		}
//
//		DtlsTransport::privateKey = PEM_read_PrivateKey(file, nullptr, nullptr, nullptr);
//
//		if (!DtlsTransport::privateKey)
//		{
//			LOG_OPENSSL_ERROR("PEM_read_PrivateKey() failed");
//
//			goto error;
//		}
//
//		fclose(file);
//
//		return;
//
//	error:
//
//		base::uv::throwError("error reading DTLS certificate and private key PEM files");
//	}

	void DtlsTransport::CreateSslCtx()
	{
		

		std::string dtlsSrtpProfiles;
		EC_KEY* ecdh{ nullptr };
		int ret;

/* Set the global DTLS context. */

// Both DTLS 1.0 and 1.2 (requires OpenSSL >= 1.1.0).
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
		DtlsTransport::sslCtx = SSL_CTX_new(DTLS_method());
// Just DTLS 1.0 (requires OpenSSL >= 1.0.1).
#elif (OPENSSL_VERSION_NUMBER >= 0x10001000L)
		DtlsTransport::sslCtx = SSL_CTX_new(DTLSv1_method());
#else
#error "too old OpenSSL version"
#endif

		if (!DtlsTransport::sslCtx)
		{
			LOG_OPENSSL_ERROR("SSL_CTX_new() failed");

			goto error;
		}

//		ret = SSL_CTX_use_certificate(DtlsTransport::sslCtx, DtlsTransport::certificate);
//
//		if (ret == 0)
//		{
//			LOG_OPENSSL_ERROR("SSL_CTX_use_certificate() failed");
//
//			goto error;
//		}

//		ret = SSL_CTX_use_PrivateKey(DtlsTransport::sslCtx, DtlsTransport::privateKey);
//
//		if (ret == 0)
//		{
//			LOG_OPENSSL_ERROR("SSL_CTX_use_PrivateKey() failed");
//
//			goto error;
//		}
                
                if (SSL_CTX_load_verify_locations(DtlsTransport::sslCtx, Settings::configuration.dtlsCertificateFile.c_str(), nullptr) != 1)
                        ERR_print_errors_fp(stderr);

                if (SSL_CTX_set_default_verify_paths(DtlsTransport::sslCtx) != 1)
                        ERR_print_errors_fp(stderr);
                    
                        /* set the local certificate from CertFile */
                if (SSL_CTX_use_certificate_file(DtlsTransport::sslCtx, Settings::configuration.dtlsCertificateFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
                    ERR_print_errors_fp(stderr);
                    abort();
                }
                
                
//                
                //if(server)
                if (1) {
                    //New lines //for server side only 

                    
                    SSL_CTX_set_default_passwd_cb_userdata(DtlsTransport::sslCtx, (void *) "12345678");
                    
                    if (SSL_CTX_use_PrivateKey_file(DtlsTransport::sslCtx, Settings::configuration.dtlsPrivateKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
                        ERR_print_errors_fp(stderr);
                        abort();
                    }
                
                    ret = SSL_CTX_check_private_key(DtlsTransport::sslCtx);

                    if (ret == 0)
                    {
                            LOG_OPENSSL_ERROR("SSL_CTX_check_private_key() failed");

                            goto error;
                    }

                
                }
                //End new lines

                        
             
		// Set options.
		SSL_CTX_set_options(
		  DtlsTransport::sslCtx,
		  SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_TICKET | SSL_OP_SINGLE_ECDH_USE |
		    SSL_OP_NO_QUERY_MTU);

		// Don't use sessions cache.
		SSL_CTX_set_session_cache_mode(DtlsTransport::sslCtx, SSL_SESS_CACHE_OFF);

		// Read always as much into the buffer as possible.
		// NOTE: This is the default for DTLS, but a bug in non latest OpenSSL
		// versions makes this call required.
		SSL_CTX_set_read_ahead(DtlsTransport::sslCtx, 1);

		SSL_CTX_set_verify_depth(DtlsTransport::sslCtx, 4);

		// Require certificate from peer.
		SSL_CTX_set_verify(
		  DtlsTransport::sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, onSslCertificateVerify);

		// Set SSL info callback.
		SSL_CTX_set_info_callback(DtlsTransport::sslCtx, onSslInfo);

		// Set ciphers.
		ret = SSL_CTX_set_cipher_list(
		  DtlsTransport::sslCtx, "ALL:!ADH:!LOW:!EXP:!MD5:!aNULL:!eNULL:@STRENGTH");

		if (ret == 0)
		{
			LOG_OPENSSL_ERROR("SSL_CTX_set_cipher_list() failed");

			goto error;
		}

// Enable ECDH ciphers.
// DOC: http://en.wikibooks.org/wiki/OpenSSL/Diffie-Hellman_parameters
// NOTE: https://code.google.com/p/chromium/issues/detail?id=406458
// NOTE: https://bugs.ruby-lang.org/issues/12324
//
// Nothing to be done in OpenSSL >= 1.1.0.
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
// For OpenSSL >= 1.0.2.
#elif (OPENSSL_VERSION_NUMBER >= 0x10002000L)
		SSL_CTX_set_ecdh_auto(DtlsTransport::sslCtx, 1);
// Older versions.
#else
		ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

		if (!ecdh)
		{
			LOG_OPENSSL_ERROR("EC_KEY_new_by_curve_name() failed");

			goto error;
		}

		if (SSL_CTX_set_tmp_ecdh(DtlsTransport::sslCtx, ecdh) != 1)
		{
			LOG_OPENSSL_ERROR("SSL_CTX_set_tmp_ecdh() failed");

			goto error;
		}

		EC_KEY_free(ecdh);
		ecdh = nullptr;
#endif

		// Set the "use_srtp" DTLS extension.
		for (auto it = DtlsTransport::srtpProfiles.begin(); it != DtlsTransport::srtpProfiles.end(); ++it)
		{
			if (it != DtlsTransport::srtpProfiles.begin())
				dtlsSrtpProfiles += ":";

			SrtpProfileMapEntry* profileEntry = std::addressof(*it);
			dtlsSrtpProfiles += profileEntry->name;
		}

		MS_DEBUG_2TAGS(dtls, srtp, "setting SRTP profiles for DTLS: ", dtlsSrtpProfiles.c_str());

		// NOTE: This function returns 0 on success.
		ret = SSL_CTX_set_tlsext_use_srtp(DtlsTransport::sslCtx, dtlsSrtpProfiles.c_str());

		if (ret != 0)
		{
			MS_ERROR("SSL_CTX_set_tlsext_use_srtp() failed when entering ", dtlsSrtpProfiles.c_str());
			LOG_OPENSSL_ERROR("SSL_CTX_set_tlsext_use_srtp() failed");

			goto error;
		}

		return;

	error:

		if (DtlsTransport::sslCtx)
		{
			SSL_CTX_free(DtlsTransport::sslCtx);
			DtlsTransport::sslCtx = nullptr;
		}

		if (ecdh)
			EC_KEY_free(ecdh);

		base::uv::throwError("SSL context creation failed");
	}

	void DtlsTransport::GenerateFingerprints()
	{
		

		for (auto& kv : DtlsTransport::string2FingerprintAlgorithm)
		{
			const std::string& algorithmString = kv.first;
			FingerprintAlgorithm algorithm     = kv.second;
			uint8_t binaryFingerprint[EVP_MAX_MD_SIZE];
			unsigned int size{ 0 };
			char hexFingerprint[(EVP_MAX_MD_SIZE * 3) + 1];
			const EVP_MD* hashFunction;
			int ret;

			switch (algorithm)
			{
				case FingerprintAlgorithm::SHA1:
					hashFunction = EVP_sha1();
					break;

				case FingerprintAlgorithm::SHA224:
					hashFunction = EVP_sha224();
					break;

				case FingerprintAlgorithm::SHA256:
					hashFunction = EVP_sha256();
					break;

				case FingerprintAlgorithm::SHA384:
					hashFunction = EVP_sha384();
					break;

				case FingerprintAlgorithm::SHA512:
					hashFunction = EVP_sha512();
					break;

				default:
					base::uv::throwError("unknown algorithm");
			}

                        X509 *x509_data = SSL_CTX_get0_certificate(DtlsTransport::sslCtx);
                       
                       
                        //X509 *x509_data = SSL_get_certificate(DtlsTransport::sslCtx);

			ret = X509_digest(x509_data, hashFunction, binaryFingerprint, &size);

			if (ret == 0)
			{
				MS_ERROR("X509_digest() failed");
				base::uv::throwError("Fingerprints generation failed");
			}

			// Convert to hexadecimal format in uppercase with colons.
			for (unsigned int i{ 0 }; i < size; ++i)
			{
				std::sprintf(hexFingerprint + (i * 3), "%.2X:", binaryFingerprint[i]);
			}
			hexFingerprint[(size * 3) - 1] = '\0';

			MS_DEBUG_TAG(dtls,algorithmString, " fingerprint: ", hexFingerprint);

			// Store it in the vector.
			DtlsTransport::Fingerprint fingerprint;

			fingerprint.algorithm = DtlsTransport::GetFingerprintAlgorithm(algorithmString);
			fingerprint.value     = hexFingerprint;

			DtlsTransport::localFingerprints.push_back(fingerprint);
		}
	}

	/* Instance methods. */

	DtlsTransport::DtlsTransport(Listener* listener) : listener(listener)
	{
		

		/* Set SSL. */

		this->ssl = SSL_new(DtlsTransport::sslCtx);

		if (!this->ssl)
		{
			LOG_OPENSSL_ERROR("SSL_new() failed");

			goto error;
		}

		// Set this as custom data.
		SSL_set_ex_data(this->ssl, 0, static_cast<void*>(this));

		this->sslBioFromNetwork = BIO_new(BIO_s_mem());

		if (!this->sslBioFromNetwork)
		{
			LOG_OPENSSL_ERROR("BIO_new() failed");

			SSL_free(this->ssl);

			goto error;
		}

		this->sslBioToNetwork = BIO_new(BIO_s_mem());

		if (!this->sslBioToNetwork)
		{
			LOG_OPENSSL_ERROR("BIO_new() failed");

			BIO_free(this->sslBioFromNetwork);
			SSL_free(this->ssl);

			goto error;
		}

		SSL_set_bio(this->ssl, this->sslBioFromNetwork, this->sslBioToNetwork);

		// Set the MTU so that we don't send packets that are too large with no fragmentation.
		SSL_set_mtu(this->ssl, DtlsMtu);
		DTLS_set_link_mtu(this->ssl, DtlsMtu);

		// Set callback handler for setting DTLS timer interval.
		DTLS_set_timer_cb(this->ssl, onSslDtlsTimer);

		// Set the DTLS timer.
		this->timer = new Timer(this);

		return;

	error:

		// NOTE: At this point SSL_set_bio() was not called so we must free BIOs as
		// well.
		if (this->sslBioFromNetwork)
			BIO_free(this->sslBioFromNetwork);

		if (this->sslBioToNetwork)
			BIO_free(this->sslBioToNetwork);

		if (this->ssl)
			SSL_free(this->ssl);

		// NOTE: If this is not catched by the caller the program will abort, but
		// this should never happen.
		base::uv::throwError("DtlsTransport instance creation failed");
	}

	DtlsTransport::~DtlsTransport()
	{
		

		if (IsRunning())
		{
			// Send close alert to the peer.
			SSL_shutdown(this->ssl);
			SendPendingOutgoingDtlsData();
		}

		if (this->ssl)
		{
			SSL_free(this->ssl);

			this->ssl               = nullptr;
			this->sslBioFromNetwork = nullptr;
			this->sslBioToNetwork   = nullptr;
		}

		// Close the DTLS timer.
		delete this->timer;
	}

	void DtlsTransport::Dump() const
	{
		

		std::string state{ "new" };
		std::string role{ "none " };

		switch (this->state)
		{
			case DtlsState::CONNECTING:
				state = "connecting";
				break;
			case DtlsState::CONNECTED:
				state = "connected";
				break;
			case DtlsState::FAILED:
				state = "failed";
				break;
			case DtlsState::CLOSED:
				state = "closed";
				break;
			default:;
		}

		switch (this->localRole)
		{
			case Role::AUTO:
				role = "auto";
				break;
			case Role::SERVER:
				role = "server";
				break;
			case Role::CLIENT:
				role = "client";
				break;
			default:;
		}

		MS_DUMP("<DtlsTransport>");
		MS_DUMP("  state           : ", state);
		MS_DUMP("  role            : ", role);
		MS_DUMP("  handshake done: : ", (this->handshakeDone ? "yes" : "no"));
		MS_DUMP("</DtlsTransport>");
	}

	void DtlsTransport::Run(Role localRole)
	{
		

		assertm(
		  localRole == Role::CLIENT || localRole == Role::SERVER,
		  "local DTLS role must be 'client' or 'server'");

		Role previousLocalRole = this->localRole;

		if (localRole == previousLocalRole)
		{
			MS_ERROR("same local DTLS role provided, doing nothing");

			return;
		}

		// If the previous local DTLS role was 'client' or 'server' do reset.
		if (previousLocalRole == Role::CLIENT || previousLocalRole == Role::SERVER)
		{
			MS_DEBUG_TAG(dtls, "resetting DTLS due to local role change");

			Reset();
		}

		// Update local role.
		this->localRole = localRole;

		// Set state and notify the listener.
		this->state = DtlsState::CONNECTING;
		this->listener->OnDtlsTransportConnecting(this);

		switch (this->localRole)
		{
			case Role::CLIENT:
			{
				MS_DEBUG_TAG(dtls, "running [role:client]");

				SSL_set_connect_state(this->ssl);
				SSL_do_handshake(this->ssl);
				SendPendingOutgoingDtlsData();
				SetTimeout();

				break;
			}

			case Role::SERVER:
			{
				MS_DEBUG_TAG(dtls, "running [role:server]");

				SSL_set_accept_state(this->ssl);
				SSL_do_handshake(this->ssl);

				break;
			}

			default:
			{
				MS_ABORT("invalid local DTLS role");
			}
		}
	}

	bool DtlsTransport::SetRemoteFingerprint(Fingerprint fingerprint)
	{
		

		assertm(
		  fingerprint.algorithm != FingerprintAlgorithm::NONE, "no fingerprint algorithm provided");

		this->remoteFingerprint = fingerprint;

		// The remote fingerpring may have been set after DTLS handshake was done,
		// so we may need to process it now.
		if (this->handshakeDone && this->state != DtlsState::CONNECTED)
		{
			MS_DEBUG_TAG(dtls, "handshake already done, processing it right now");

			return ProcessHandshake();
		}

		return true;
	}

	void DtlsTransport::ProcessDtlsData(const uint8_t* data, size_t len)
	{
		

		int written;
		int read;

		if (!IsRunning())
		{
			MS_ERROR("cannot process data while not running");

			return;
		}

		// Write the received DTLS data into the sslBioFromNetwork.
		written =
		  BIO_write(this->sslBioFromNetwork, static_cast<const void*>(data), static_cast<int>(len));

		if (written != static_cast<int>(len))
		{
			MS_WARN_TAG( dtls, "OpenSSL BIO_write() wrote less ", static_cast<size_t>(written), " than given data ", len);
			 
		}

		// Must call SSL_read() to process received DTLS data.
		read = SSL_read(this->ssl, static_cast<void*>(DtlsTransport::sslReadBuffer), SslReadBufferSize);

		// Send data if it's ready.
		SendPendingOutgoingDtlsData();

		// Check SSL status and return if it is bad/closed.
		if (!CheckStatus(read))
			return;

		// Set/update the DTLS timeout.
		if (!SetTimeout())
			return;

		// Application data received. Notify to the listener.
		if (read > 0)
		{
			// It is allowed to receive DTLS data even before validating remote fingerprint.
			if (!this->handshakeDone)
			{
				MS_WARN_TAG(dtls, "ignoring application data received while DTLS handshake not done");

				return;
			}

			// Notify the listener.
			this->listener->OnDtlsTransportApplicationDataReceived(
			  this, (uint8_t*)DtlsTransport::sslReadBuffer, static_cast<size_t>(read));
		}
	}

	void DtlsTransport::SendApplicationData(const uint8_t* data, size_t len)
	{
		

		// We cannot send data to the peer if its remote fingerprint is not validated.
		if (this->state != DtlsState::CONNECTED)
		{
			MS_WARN_TAG(dtls, "cannot send application data while DTLS is not fully connected");

			return;
		}

		if (len == 0)
		{
			MS_WARN_TAG(dtls, "ignoring 0 length data");

			return;
		}

		int written;

		written = SSL_write(this->ssl, static_cast<const void*>(data), static_cast<int>(len));

		if (written < 0)
		{
			LOG_OPENSSL_ERROR("SSL_write() failed");

			if (!CheckStatus(written))
				return;
		}
		else if (written != static_cast<int>(len))
		{
			MS_WARN_TAG(  dtls, "OpenSSL SSL_write() wrote less ", written, " than given data bytes ", len);
		}

		// Send data.
		SendPendingOutgoingDtlsData();
	}

	void DtlsTransport::Reset()
	{
		

		int ret;

		if (!IsRunning())
			return;

		MS_WARN_TAG(dtls, "resetting DTLS transport");

		// Stop the DTLS timer.
		this->timer->Stop();

		// We need to reset the SSL instance so we need to "shutdown" it, but we
		// don't want to send a Close Alert to the peer, so just don't call
		// SendPendingOutgoingDTLSData().
		SSL_shutdown(this->ssl);

		this->localRole        = Role::NONE;
		this->state            = DtlsState::NEW;
		this->handshakeDone    = false;
		this->handshakeDoneNow = false;

		// Reset SSL status.
		// NOTE: For this to properly work, SSL_shutdown() must be called before.
		// NOTE: This may fail if not enough DTLS handshake data has been received,
		// but we don't care so just clear the error queue.
		ret = SSL_clear(this->ssl);

		if (ret == 0)
			ERR_clear_error();
	}

	inline bool DtlsTransport::CheckStatus(int returnCode)
	{
		

		int err;
		bool wasHandshakeDone = this->handshakeDone;

		err = SSL_get_error(this->ssl, returnCode);

		switch (err)
		{
			case SSL_ERROR_NONE:
				break;

			case SSL_ERROR_SSL:
				LOG_OPENSSL_ERROR("SSL status: SSL_ERROR_SSL");
				break;

			case SSL_ERROR_WANT_READ:
				break;

			case SSL_ERROR_WANT_WRITE:
				MS_WARN_TAG(dtls, "SSL status: SSL_ERROR_WANT_WRITE");
				break;

			case SSL_ERROR_WANT_X509_LOOKUP:
				MS_DEBUG_TAG(dtls, "SSL status: SSL_ERROR_WANT_X509_LOOKUP");
				break;

			case SSL_ERROR_SYSCALL:
				LOG_OPENSSL_ERROR("SSL status: SSL_ERROR_SYSCALL");
				break;

			case SSL_ERROR_ZERO_RETURN:
				break;

			case SSL_ERROR_WANT_CONNECT:
				MS_WARN_TAG(dtls, "SSL status: SSL_ERROR_WANT_CONNECT");
				break;

			case SSL_ERROR_WANT_ACCEPT:
				MS_WARN_TAG(dtls, "SSL status: SSL_ERROR_WANT_ACCEPT");
				break;

			default:
				MS_WARN_TAG(dtls, "SSL status: unknown error");
		}

		// Check if the handshake (or re-handshake) has been done right now.
		if (this->handshakeDoneNow)
		{
			this->handshakeDoneNow = false;
			this->handshakeDone    = true;

			// Stop the timer.
			this->timer->Stop();

			// Process the handshake just once (ignore if DTLS renegotiation).
			if (!wasHandshakeDone && this->remoteFingerprint.algorithm != FingerprintAlgorithm::NONE)
				return ProcessHandshake();

			return true;
		}
		// Check if the peer sent close alert or a fatal error happened.
		else if (((SSL_get_shutdown(this->ssl) & SSL_RECEIVED_SHUTDOWN) != 0) || err == SSL_ERROR_SSL || err == SSL_ERROR_SYSCALL)
		{
			if (this->state == DtlsState::CONNECTED)
			{
				MS_DEBUG_TAG(dtls, "disconnected");

				Reset();

				// Set state and notify the listener.
				this->state = DtlsState::CLOSED;
				this->listener->OnDtlsTransportClosed(this);
			}
			else
			{
				MS_WARN_TAG(dtls, "connection failed");

				Reset();

				// Set state and notify the listener.
				this->state = DtlsState::FAILED;
				this->listener->OnDtlsTransportFailed(this);
			}

			return false;
		}
		else
		{
			return true;
		}
	}

	inline void DtlsTransport::SendPendingOutgoingDtlsData()
	{
		

		if (BIO_eof(this->sslBioToNetwork))
			return;

		int64_t read;
		char* data{ nullptr };

		read = BIO_get_mem_data(this->sslBioToNetwork, &data); // NOLINT

		if (read <= 0)
			return;

		//SDebug << read << " bytes of DTLS data ready to sent to the peer" ;

		// Notify the listener.
		this->listener->OnDtlsTransportSendData(
		  this, reinterpret_cast<uint8_t*>(data), static_cast<size_t>(read));

		// Clear the BIO buffer.
		// NOTE: the (void) avoids the -Wunused-value warning.
		(void)BIO_reset(this->sslBioToNetwork);
	}

	inline bool DtlsTransport::SetTimeout()
	{
		

		assertm(
		  this->state == DtlsState::CONNECTING || this->state == DtlsState::CONNECTED,
		  "invalid DTLS state");

		int64_t ret;
		uv_timeval_t dtlsTimeout{ 0, 0 };
		uint64_t timeoutMs;

		// NOTE: If ret == 0 then ignore the value in dtlsTimeout.
		// NOTE: No DTLSv_1_2_get_timeout() or DTLS_get_timeout() in OpenSSL 1.1.0-dev.
		ret = DTLSv1_get_timeout(this->ssl, static_cast<void*>(&dtlsTimeout)); // NOLINT

		if (ret == 0)
			return true;

		timeoutMs = (dtlsTimeout.tv_sec * static_cast<uint64_t>(1000)) + (dtlsTimeout.tv_usec / 1000);

		if (timeoutMs == 0)
		{
			return true;
		}
		else if (timeoutMs < 30000)
		{
			LDebug("DTLS timer set in ms", timeoutMs);

			this->timer->Start(timeoutMs);

			return true;
		}
		// NOTE: Don't start the timer again if the timeout is greater than 30 seconds.
		else
		{
			MS_WARN_TAG(dtls, "DTLS timeout too high (%" PRIu64 "ms), resetting DLTS", timeoutMs);

			Reset();

			// Set state and notify the listener.
			this->state = DtlsState::FAILED;
			this->listener->OnDtlsTransportFailed(this);

			return false;
		}
	}

	inline bool DtlsTransport::ProcessHandshake()
	{
		

		assertm(this->handshakeDone, "handshake not done yet");
		assertm(
		  this->remoteFingerprint.algorithm != FingerprintAlgorithm::NONE, "remote fingerprint not set");

		// Validate the remote fingerprint.
		if (!CheckRemoteFingerprint())
		{
			Reset();

			// Set state and notify the listener.
			this->state = DtlsState::FAILED;
			this->listener->OnDtlsTransportFailed(this);

			return false;
		}

		// Get the negotiated SRTP profile.
		RTC::SrtpSession::Profile srtpProfile = GetNegotiatedSrtpProfile();

		if (srtpProfile != RTC::SrtpSession::Profile::NONE)
		{
			// Extract the SRTP keys (will notify the listener with them).
			ExtractSrtpKeys(srtpProfile);

			return true;
		}

		// NOTE: We assume that "use_srtp" DTLS extension is required even if
		// there is no audio/video.
		MS_WARN_2TAGS(dtls, srtp, "SRTP profile not negotiated");

		Reset();

		// Set state and notify the listener.
		this->state = DtlsState::FAILED;
		this->listener->OnDtlsTransportFailed(this);

		return false;
	}

	inline bool DtlsTransport::CheckRemoteFingerprint()
	{
		

		assertm(
		  this->remoteFingerprint.algorithm != FingerprintAlgorithm::NONE, "remote fingerprint not set");

		X509* certificate;
		uint8_t binaryFingerprint[EVP_MAX_MD_SIZE];
		unsigned int size{ 0 };
		char hexFingerprint[(EVP_MAX_MD_SIZE * 3) + 1];
		const EVP_MD* hashFunction;
		int ret;

		certificate = SSL_get_peer_certificate(this->ssl);

		if (!certificate)
		{
			MS_WARN_TAG(dtls, "no certificate was provided by the peer");

			return false;
		}

		switch (this->remoteFingerprint.algorithm)
		{
			case FingerprintAlgorithm::SHA1:
				hashFunction = EVP_sha1();
				break;

			case FingerprintAlgorithm::SHA224:
				hashFunction = EVP_sha224();
				break;

			case FingerprintAlgorithm::SHA256:
				hashFunction = EVP_sha256();
				break;

			case FingerprintAlgorithm::SHA384:
				hashFunction = EVP_sha384();
				break;

			case FingerprintAlgorithm::SHA512:
				hashFunction = EVP_sha512();
				break;

			default:
				MS_ABORT("unknown algorithm");
		}

		// Compare the remote fingerprint with the value given via signaling.
		ret = X509_digest(certificate, hashFunction, binaryFingerprint, &size);

		if (ret == 0)
		{
			MS_ERROR("X509_digest() failed");

			X509_free(certificate);

			return false;
		}

		// Convert to hexadecimal format in uppercase with colons.
		for (unsigned int i{ 0 }; i < size; ++i)
		{
			std::sprintf(hexFingerprint + (i * 3), "%.2X:", binaryFingerprint[i]);
		}
		hexFingerprint[(size * 3) - 1] = '\0';

		if (this->remoteFingerprint.value != hexFingerprint)
		{
			MS_WARN_TAG(
			  dtls,
			  "fingerprint in the remote certificate (%s) does not match the announced one (%s)",
			  hexFingerprint,
			  this->remoteFingerprint.value.c_str());

			X509_free(certificate);

			return false;
		}

		MS_DEBUG_TAG(dtls, "valid remote fingerprint");

		// Get the remote certificate in PEM format.

		BIO* bio = BIO_new(BIO_s_mem());

		// Ensure the underlying BUF_MEM structure is also freed.
		// NOTE: Avoid stupid "warning: value computed is not used [-Wunused-value]" since
		// BIO_set_close() always returns 1.
		(void)BIO_set_close(bio, BIO_CLOSE);

		ret = PEM_write_bio_X509(bio, certificate);

		if (ret != 1)
		{
			LOG_OPENSSL_ERROR("PEM_write_bio_X509() failed");

			X509_free(certificate);
			BIO_free(bio);

			return false;
		}

		BUF_MEM* mem;

		BIO_get_mem_ptr(bio, &mem); // NOLINT[cppcoreguidelines-pro-type-cstyle-cast]

		if (!mem || !mem->data || mem->length == 0u)
		{
			LOG_OPENSSL_ERROR("BIO_get_mem_ptr() failed");

			X509_free(certificate);
			BIO_free(bio);

			return false;
		}

		this->remoteCert = std::string(mem->data, mem->length);

		X509_free(certificate);
		BIO_free(bio);

		return true;
	}

	inline void DtlsTransport::ExtractSrtpKeys(RTC::SrtpSession::Profile srtpProfile)
	{
		

		size_t srtpKeyLength{ 0 };
		size_t srtpSaltLength{ 0 };
		size_t srtpMasterLength{ 0 };

		switch (srtpProfile)
		{
			case RTC::SrtpSession::Profile::AES_CM_128_HMAC_SHA1_80:
			case RTC::SrtpSession::Profile::AES_CM_128_HMAC_SHA1_32:
			{
				srtpKeyLength    = SrtpMasterKeyLength;
				srtpSaltLength   = SrtpMasterSaltLength;
				srtpMasterLength = SrtpMasterLength;

				break;
			}

			case RTC::SrtpSession::Profile::AEAD_AES_256_GCM:
			{
				srtpKeyLength    = SrtpAesGcm256MasterKeyLength;
				srtpSaltLength   = SrtpAesGcm256MasterSaltLength;
				srtpMasterLength = SrtpAesGcm256MasterLength;

				break;
			}

			case RTC::SrtpSession::Profile::AEAD_AES_128_GCM:
			{
				srtpKeyLength    = SrtpAesGcm128MasterKeyLength;
				srtpSaltLength   = SrtpAesGcm128MasterSaltLength;
				srtpMasterLength = SrtpAesGcm128MasterLength;

				break;
			}

			default:
			{
				MS_ABORT("unknown SRTP profile");
			}
		}

		auto* srtpMaterial = new uint8_t[srtpMasterLength * 2];
		uint8_t* srtpLocalKey{ nullptr };
		uint8_t* srtpLocalSalt{ nullptr };
		uint8_t* srtpRemoteKey{ nullptr };
		uint8_t* srtpRemoteSalt{ nullptr };
		auto* srtpLocalMasterKey  = new uint8_t[srtpMasterLength];
		auto* srtpRemoteMasterKey = new uint8_t[srtpMasterLength];
		int ret;

		ret = SSL_export_keying_material(
		  this->ssl, srtpMaterial, srtpMasterLength * 2, "EXTRACTOR-dtls_srtp", 19, nullptr, 0, 0);

		assertm(ret != 0, "SSL_export_keying_material() failed");

		switch (this->localRole)
		{
			case Role::SERVER:
			{
				srtpRemoteKey  = srtpMaterial;
				srtpLocalKey   = srtpRemoteKey + srtpKeyLength;
				srtpRemoteSalt = srtpLocalKey + srtpKeyLength;
				srtpLocalSalt  = srtpRemoteSalt + srtpSaltLength;

				break;
			}

			case Role::CLIENT:
			{
				srtpLocalKey   = srtpMaterial;
				srtpRemoteKey  = srtpLocalKey + srtpKeyLength;
				srtpLocalSalt  = srtpRemoteKey + srtpKeyLength;
				srtpRemoteSalt = srtpLocalSalt + srtpSaltLength;

				break;
			}

			default:
			{
				MS_ABORT("no DTLS role set");
			}
		}

		// Create the SRTP local master key.
		std::memcpy(srtpLocalMasterKey, srtpLocalKey, srtpKeyLength);
		std::memcpy(srtpLocalMasterKey + srtpKeyLength, srtpLocalSalt, srtpSaltLength);
		// Create the SRTP remote master key.
		std::memcpy(srtpRemoteMasterKey, srtpRemoteKey, srtpKeyLength);
		std::memcpy(srtpRemoteMasterKey + srtpKeyLength, srtpRemoteSalt, srtpSaltLength);

		// Set state and notify the listener.
		this->state = DtlsState::CONNECTED;
		this->listener->OnDtlsTransportConnected(
		  this,
		  srtpProfile,
		  srtpLocalMasterKey,
		  srtpMasterLength,
		  srtpRemoteMasterKey,
		  srtpMasterLength,
		  this->remoteCert);

		delete[] srtpMaterial;
		delete[] srtpLocalMasterKey;
		delete[] srtpRemoteMasterKey;
	}

	inline RTC::SrtpSession::Profile DtlsTransport::GetNegotiatedSrtpProfile()
	{
		

		RTC::SrtpSession::Profile negotiatedSrtpProfile = RTC::SrtpSession::Profile::NONE;

		// Ensure that the SRTP profile has been negotiated.
		SRTP_PROTECTION_PROFILE* sslSrtpProfile = SSL_get_selected_srtp_profile(this->ssl);

		if (!sslSrtpProfile)
			return negotiatedSrtpProfile;

		// Get the negotiated SRTP profile.
		for (auto& srtpProfile : DtlsTransport::srtpProfiles)
		{
			SrtpProfileMapEntry* profileEntry = std::addressof(srtpProfile);

			if (std::strcmp(sslSrtpProfile->name, profileEntry->name) == 0)
			{
				MS_DEBUG_2TAGS(dtls, srtp, "chosen SRTP profile: ", profileEntry->name);

				negotiatedSrtpProfile = profileEntry->profile;
			}
		}

		assertm(
		  negotiatedSrtpProfile != RTC::SrtpSession::Profile::NONE,
		  "chosen SRTP profile is not an available one");

		return negotiatedSrtpProfile;
	}

	inline void DtlsTransport::OnSslInfo(int where, int ret)
	{
		

		int w = where & -SSL_ST_MASK;
		const char* role;

		if ((w & SSL_ST_CONNECT) != 0)
			role = "client";
		else if ((w & SSL_ST_ACCEPT) != 0)
			role = "server";
		else
			role = "undefined";

		if ((where & SSL_CB_LOOP) != 0)
		{
			MS_DEBUG_TAG(dtls, "role: ", role," action: ", SSL_state_string_long(this->ssl));
		}
		else if ((where & SSL_CB_ALERT) != 0)
		{
			const char* alertType;

			switch (*SSL_alert_type_string(ret))
			{
				case 'W':
					alertType = "warning";
					break;

				case 'F':
					alertType = "fatal";
					break;

				default:
					alertType = "undefined";
			}

			if ((where & SSL_CB_READ) != 0)
			{
				MS_WARN_TAG(dtls, "received DTLS ",alertType, " alert: ",  SSL_alert_desc_string_long(ret));
			}
			else if ((where & SSL_CB_WRITE) != 0)
			{
				MS_DEBUG_TAG(dtls, "sending DTLS ", alertType, " alert: ",  SSL_alert_desc_string_long(ret));
			}
			else
			{
				MS_DEBUG_TAG(dtls, "DTLS ", alertType, " alert: ",  SSL_alert_desc_string_long(ret));
			}
		}
		else if ((where & SSL_CB_EXIT) != 0)
		{
			if (ret == 0)
                        {
				MS_DEBUG_TAG(dtls, "role: ", role, " failed: ",  SSL_state_string_long(this->ssl));
                        }
			else if (ret < 0)
                        {
				MS_DEBUG_TAG(dtls, "role: ", role, " waiting: ",  SSL_state_string_long(this->ssl));
                        }
		}
		else if ((where & SSL_CB_HANDSHAKE_START) != 0)
		{
			MS_DEBUG_TAG(dtls, "DTLS handshake start");
		}
		else if ((where & SSL_CB_HANDSHAKE_DONE) != 0)
		{
			MS_DEBUG_TAG(dtls, "DTLS handshake done");

			this->handshakeDoneNow = true;
		}

		// NOTE: checking SSL_get_shutdown(this->ssl) & SSL_RECEIVED_SHUTDOWN here upon
		// receipt of a close alert does not work (the flag is set after this callback).
	}

	inline void DtlsTransport::OnTimer(Timer* /*timer*/)
	{

		// Workaround for https://github.com/openssl/openssl/issues/7998.
		if (this->handshakeDone)
		{
			LDebug("handshake is done so return");

			return;
		}

		DTLSv1_handle_timeout(this->ssl);

		// If required, send DTLS data.
		SendPendingOutgoingDtlsData();

		// Set the DTLS timer again.
		SetTimeout();
	}
} // namespace RTC
