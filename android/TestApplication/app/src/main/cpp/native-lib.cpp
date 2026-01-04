#include <jni.h>
#include <string>
#include <ios>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/types.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>


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
    std::string b64 =  base64_encode_by_bio(hello);
    return env->NewStringUTF(b64.c_str());
}