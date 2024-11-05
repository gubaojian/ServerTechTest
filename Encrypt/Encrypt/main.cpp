//
//  main.cpp
//  Encrypt
//
//  Created by liz on 2024/11/5.
//

#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <boost/algorithm/hex.hpp>


//https://www.cnblogs.com/cocoajin/p/6134382.html

int main(int argc, const char * argv[]) {
    
    EVP_PKEY* rsaKeyPair = EVP_PKEY_new();
    EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new_id( EVP_PKEY_RSA, NULL );
    EVP_PKEY_keygen_init( ctx );
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024);
    // could set parameters here
    EVP_PKEY_keygen(ctx, &rsaKeyPair);
    

    
    // 设置加密上下文
    EVP_PKEY_CTX *ectx;
    ectx = EVP_PKEY_CTX_new(rsaKeyPair, NULL);
    EVP_PKEY_encrypt_init(ectx);
  
    std::string hello = "hello worldhello worl";
    char buffer[1024];
    size_t outLen = 0;
    int result =   EVP_PKEY_encrypt(ectx, (unsigned char *)buffer, &outLen, (const unsigned char *)hello.data(), hello.length());
    if (result == -1) {
        std::cout << "encrypt error " << std::endl;
    }
    std::cout << "hello world encrypt length " << outLen << std::endl;
    
    
    EVP_PKEY_CTX *dctx;
    dctx = EVP_PKEY_CTX_new(rsaKeyPair, NULL);
    EVP_PKEY_decrypt_init(dctx);
    char buffer2[1024];
    size_t dLen = 20;
    result = EVP_PKEY_decrypt(dctx, (unsigned char *)buffer2, &dLen, (unsigned char *)buffer, outLen);
    if (result == -1) {
        std::cout << "decrypt error " << std::endl;
    }
    std::cout << "decrypt success " << dLen << std::endl;
    
    std::cout << "decrypt content " << std::string(buffer2, dLen) << std::endl;
    
    
    
    
    EVP_PKEY_CTX_free(ectx);
    EVP_PKEY_CTX_free(dctx);
    
    
    // ...
    EVP_PKEY_CTX_free(ctx);
    
    std::cout << hello << std::endl;
    return 0;
}
