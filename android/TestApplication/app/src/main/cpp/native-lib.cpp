#include <jni.h>
#include <string>
#include <ios>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/types.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <string_view>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#ifndef OSSL_PKEY_RSA_PAD_MODE_PKCS1
#define OSSL_PKEY_RSA_PAD_MODE_PKCS1 "pkcs1"
#endif

#define CAMEL_KEY_FORMAT_BASE64  "base64"
#define CAMEL_KEY_FORMAT_HEX  "hex"
#define CAMEL_KEY_FORMAT_PEM  "pem"
#define CAMEL_KEY_FORMAT_DER  "der"
#define CAMEL_KEY_FORMAT_BINARY  "binary"
#define CAMEL_KEY_FORMAT_RAW  "raw"

#define CHECK_SIGN_USE_CRYPTO_MEMCMP  false

#define CAMEL_EVP_KEY_CACHE_SIZE  8

//#define CAMEL_DEBUG_LOG  1

namespace camel{
    namespace crypto {
        void initLibCrypto();
        void cleanupLibCrypto();
        void printOpenSSLError();

        OSSL_LIB_CTX *getOSSL_LIB_CTX();

        bool fast_cmp_equals(std::string_view now_sign, std::string_view expect_sign);

    }
}

namespace camel {
    namespace crypto {
        std::string hex_encode(const std::string &input);
        std::string hex_encode(const std::string_view &input);
        std::string hex_encode_lower(const std::string_view &input);
    }
}

#include <iostream>
#include <string>

#include <openssl/bn.h>

namespace camel {
    namespace crypto {
        inline std::string hex_encode_by_bn(const std::string &input) {
            std::string result;
            BIGNUM *bn = BN_new();
            if (bn == nullptr) {
                return result;
            }
            result.reserve(input.size() * 2 + 4);
            BN_bin2bn((unsigned char*)input.data(), input.length(), bn);
            char *hex = BN_bn2hex(bn);
            if (hex != nullptr) {
                result.append(hex);
                OPENSSL_free(hex);
            }
            BN_free(bn);
            return result;
        }
        static constexpr char kEncodingLower[] = "0123456789abcdef";

        static constexpr char kEncodingUpper[] = "0123456789ABCDEF";

        inline std::string hex_encode_by_block(const std::string_view& input, const char* kEncoding) {
            const size_t input_len = input.size();
            if (input_len == 0) {
                return "";
            }
            std::string result(input.size() * 2, '\0');
            char* out = result.data();  // 直接操作缓冲区指针
            const uint8_t* in = (const uint8_t*)(input.data());

            // 批量处理：每次处理4字节（生成8个十六进制字符），减少循环次数
            size_t i = 0;
            const size_t can_batch_length = input_len - (input_len % 4);  // 4的倍数部分
            for (; i < can_batch_length; i += 4) {
                uint8_t b0 = in[i], b1 = in[i+1], b2 = in[i+2], b3 = in[i+3];
                out[0] = kEncoding[b0 >> 4];
                out[1] = kEncoding[b0 & 0xF];
                out[2] = kEncoding[b1 >> 4];
                out[3] = kEncoding[b1 & 0xF];
                out[4] = kEncoding[b2 >> 4];
                out[5] = kEncoding[b2 & 0xF];
                out[6] = kEncoding[b3 >> 4];
                out[7] = kEncoding[b3 & 0xF];
                out += 8;
            }

            // 处理剩余字节（不足4字节的部分）
            for (; i < input_len; ++i) {
                uint8_t b = in[i];
                *out++ = kEncoding[b >> 4];
                *out++ = kEncoding[b & 0xF];
            }
            return result;
        }



        std::string hex_decode_by_bn(const std::string_view &input) {
            std::string result;
            BIGNUM *bn = BN_new();
            if (bn == nullptr) {
                return result;
            }
            if (BN_hex2bn(&bn, input.data()) == 0) {
                BN_free(bn);
                return result;
            }

            int expected_len = input.length()/2;
            result.resize(expected_len);
            unsigned char* out = (unsigned char*)(result.data());
            BN_bn2binpad(bn, out, expected_len);
            BN_free(bn);
            return result;
        }




        std::string hex_encode(const std::string &input) {
            return hex_encode_by_block(input, kEncodingUpper);
        }

        std::string hex_encode(const std::string_view &input) {
            return hex_encode_by_block(input, kEncodingUpper);
        }

        std::string hex_encode_lower(const std::string_view &input) {
            return hex_encode_by_block(input, kEncodingLower);
        }



    }
}

namespace camel {
    namespace crypto {
        /*
        * @brief 标准 Base64 编码（无换行符）
        * @param input 待编码的原始二进制数据
        * @return 编码后的 Base64 字符串（包含 +、/，可能包含 = 填充符，无换行）
        */
        std::string base64_encode(const std::string &input);
    }
}

namespace camel {
    namespace crypto {
        inline std::string base64_encode_by_bio(const std::string &input) {
            if (input.empty()) {
                return "";
            }
            BIO *bio, *b64;
            // Create a base64 filter BIO
            b64 = BIO_new(BIO_f_base64());
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

            bio = BIO_new(BIO_s_mem());

            BIO_push(b64, bio);
            BIO_write(b64, input.data(), input.length());

            BIO_flush(b64);
            char *output_data = nullptr;
            long output_len = BIO_get_mem_data(bio, &output_data); // 获取内存指针和长度
            std::string result(output_data, output_len);
            // Free all BIOs in the chain
            BIO_free_all(b64);
            return result;
        }

        /**
         * 无换行，速度比base64_encode_by_bio快3-4倍
         * @param input
         * @return
         */
        inline std::string base64_encode_by_block(const std::string &input) {
            if (input.empty()) {
                return "";
            }

            std::string result;
            result.resize(input.length() * 2);
            unsigned char *out = (unsigned char *) result.data();;
            unsigned char *in = (unsigned char *) input.data();
            int encoded_len = EVP_EncodeBlock(out, in, input.size());
            if (encoded_len <= 0) {
                return "";
            }
            result.resize(encoded_len);
            return result;
        }

        std::string base64_encode(const std::string &input) {
            return base64_encode_by_block(input);
        }
    }
}

namespace camel {
    namespace crypto {

        class MessageDigest {
        public:
            /**
            *  *  Known DIGEST names (not a complete list)
            *  # define OSSL_DIGEST_NAME_MD5            "MD5"
            *  # define OSSL_DIGEST_NAME_MD5_SHA1       "MD5-SHA1"
            *  # define OSSL_DIGEST_NAME_SHA1           "SHA1"
            *  # define OSSL_DIGEST_NAME_SHA2_224       "SHA2-224"
            *  # define OSSL_DIGEST_NAME_SHA2_256       "SHA2-256"
            *  # define OSSL_DIGEST_NAME_SHA2_256_192   "SHA2-256/192"
            *  # define OSSL_DIGEST_NAME_SHA2_384       "SHA2-384"
            *  # define OSSL_DIGEST_NAME_SHA2_512       "SHA2-512"
            *  # define OSSL_DIGEST_NAME_SHA2_512_224   "SHA2-512/224"
            *  # define OSSL_DIGEST_NAME_SHA2_512_256   "SHA2-512/256"
            *  # define OSSL_DIGEST_NAME_RIPEMD160      "RIPEMD160"
            *  # define OSSL_DIGEST_NAME_SHA3_224       "SHA3-224"
            *  # define OSSL_DIGEST_NAME_SHA3_256       "SHA3-256"
            *  # define OSSL_DIGEST_NAME_SHA3_384       "SHA3-384"
            *  # define OSSL_DIGEST_NAME_SHA3_512       "SHA3-512"
            *  # define OSSL_DIGEST_NAME_SM3            "SM3"
             * @param algorithm
             */
            explicit MessageDigest(const std::string &algorithm);

        public:
            std::string digest(const std::string_view &data);

            std::string digestToHex(const std::string_view &data);

            std::string digestToBase64(const std::string_view &data);

        private:
            std::string algorithm; //MD5 SHA2-256
        };


    }
}

namespace camel{
    namespace crypto {
        bool _hasOpenSslInit = false;

        void initLibCrypto() {
            if (!_hasOpenSslInit) {
                OpenSSL_add_all_algorithms();
                ERR_load_crypto_strings();
                _hasOpenSslInit = true;
            }
        }

        void printOpenSSLError() {
            unsigned long errCode;
            char errMsg[256];

            while ((errCode = ERR_get_error()) != 0) {
                ERR_error_string_n(errCode, errMsg, sizeof(errMsg));
                std::cerr << "OpenSSL Error: " << errMsg << std::endl;
            }
        }

        void cleanupLibCrypto() {
            if (_hasOpenSslInit) {
                EVP_cleanup();
                ERR_free_strings();
                _hasOpenSslInit = false;
            }
        }

        OSSL_LIB_CTX *getOSSL_LIB_CTX() {
            return nullptr;
        }

        bool fast_cmp_equals(std::string_view now_sign, std::string_view expect_sign) {
            if (now_sign.length() != expect_sign.size()) {
                return false;
            }
            // none need use CRYPTO_memcmp, just fast compare is ok.
            if (CHECK_SIGN_USE_CRYPTO_MEMCMP) {
                return CRYPTO_memcmp(now_sign.data(), expect_sign.data(), now_sign.size()) == 0;
            }
            return std::memcmp(now_sign.data(), expect_sign.data(), now_sign.size()) == 0;
        }
    }
}

namespace camel {
    namespace crypto {
        MessageDigest::MessageDigest(const std::string &algorithm) {
            this->algorithm = algorithm;
        }

        std::string MessageDigest::digest(const std::string_view& data) {
            OSSL_LIB_CTX *libctx = getOSSL_LIB_CTX();
            EVP_MD *md = EVP_MD_fetch(libctx, algorithm.data(), NULL);
            if (md == nullptr) {
                std::cerr << "MessageDigest::digest() Failed to EVP_MD_fetch " << algorithm << std::endl;
                printOpenSSLError();
                return "";
            }

            EVP_MD_CTX *ctx = EVP_MD_CTX_new();
            if (ctx == nullptr) {
                std::cerr << "MessageDigest::digest() Failed to EVP_MD_CTX_new() " << std::endl;
                printOpenSSLError();
                EVP_MD_free(md);
                return "";
            }

            if (EVP_DigestInit_ex2(ctx, md, NULL) != 1) {
                std::cerr << "MessageDigest::digest() Failed to EVP_DigestInit_ex2() " << std::endl;
                printOpenSSLError();
                EVP_MD_CTX_free(ctx);
                EVP_MD_free(md);
                return "";
            }

            if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
                std::cerr << "MessageDigest::digest() Failed to EVP_DigestUpdate() " << std::endl;
                printOpenSSLError();
                EVP_MD_CTX_free(ctx);
                EVP_MD_free(md);
                return "";
            }
            std::string buffer(EVP_MAX_MD_SIZE, '\0');
            unsigned char *out = (unsigned char *) buffer.data();
            unsigned int outlen = buffer.size();
            if (EVP_DigestFinal_ex(ctx, out, &outlen) != 1) {
                std::cerr << "MessageDigest::digest() Failed to EVP_DigestFinal_ex() " << std::endl;
                printOpenSSLError();
                EVP_MD_CTX_free(ctx);
                EVP_MD_free(md);
                return "";
            }
            buffer.resize(outlen);
            EVP_MD_CTX_free(ctx);
            EVP_MD_free(md);
            return buffer;
        }


        std::string MessageDigest::digestToHex(const std::string_view& data) {
            return hex_encode_lower(digest(data));
        }

        std::string MessageDigest::digestToBase64(const std::string_view& data) {
            return base64_encode(digest(data));
        }


    }
}

namespace camel {
    namespace crypto {

        inline std::string digest(const std::string &algorithm, const std::string_view &data) {
            MessageDigest digest(algorithm);
            return digest.digest(data);
        }

        inline std::string digestToHex(const std::string &algorithm, const std::string_view &data) {
            MessageDigest digest(algorithm);
            return digest.digestToHex(data);
        }

        inline std::string
        digestToBase64(const std::string &algorithm, const std::string_view &data) {
            MessageDigest digest(algorithm);
            return digest.digestToBase64(data);
        }

        std::string md5(const std::string_view& data) {
            return digest("MD5", data);
        }
        std::string md5ToHex(const std::string_view& data) {
            return digestToHex("MD5", data);
        }
        std::string md5ToBase64(const std::string_view& data) {
            return digestToBase64("MD5", data);
        }
    }
}


inline std::string base64_encode_by_bio(const std::string &input) {
    if (input.empty()) {
        return "";
    }
    BIO *bio, *b64;
    // Create a base64 filter BIO
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new(BIO_s_mem());

    BIO_push(b64, bio);
    BIO_write(b64, input.data(), input.length());

    BIO_flush(b64);
    char* output_data = nullptr;
    long output_len = BIO_get_mem_data(bio, &output_data); // 获取内存指针和长度
    std::string result(output_data, output_len);
    // Free all BIOs in the chain
    BIO_free_all(b64);
    return result;
}

bool _hasOpenSslInit = false;

void initLibCrypto() {
    if (!_hasOpenSslInit) {
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        _hasOpenSslInit = true;
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_furture_testapplication_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from++";
    initLibCrypto();
    std::string b64 =  base64_encode_by_bio(hello) + "\nbase64"
            + "\n" + camel::crypto::md5ToBase64(hello);
    return env->NewStringUTF(b64.c_str());
}