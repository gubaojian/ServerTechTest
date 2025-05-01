//
//  map_find_test.cc
//  cplus
//
//  Created by efurture on 2024/9/26.
//

#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>
#include <boost/beast/core/detail/base64.hpp>

#include "lib/yyjson/yyjson.h"


#include <iostream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>



/**
 debug  mode
 
 

 
 release mode
  
 
 https://github.com/ibireme/yyjson
 
 https://github.com/simdjson/simdjson?tab=readme-ov-file#quick-start
 
 */



int encrypt_test_main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string publicKeyPem = R"(
    -----BEGIN PUBLIC KEY-----
    MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDlOJu6TyygqxfWT7eLtGDwajtN
    FOb9I5XRb6khyfD1Yt3YiCgQWMNW649887VGJiGr/L5i2osbl8C9+WJTeucF+S76
    xFxdU6jE0NQ+Z+zEdhUTooNRaY5nZiu5PgDB0ED/ZKBUSLKL7eibMxZtMlUDHjm4
    gwQco1KRMDSmXSMkDwIDAQAB
    -----END PUBLIC KEY-----
    )";
    
    std::string key ="MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDlOJu6TyygqxfWT7eLtGDwajtNFOb9I5XRb6khyfD1Yt3YiCgQWMNW649887VGJiGr/L5i2osbl8C9+WJTeucF+S76xFxdU6jE0NQ+Z+zEdhUTooNRaY5nZiu5PgDB0ED/ZKBUSLKL7eibMxZtMlUDHjm4gwQco1KRMDSmXSMkDwIDAQAB";
    std::string dest;
    dest.resize(512);
    auto pair = boost::beast::detail::base64::decode(dest.data(), key.data(), key.size());
    
    std::cout << pair.first << "  " << pair.second << std::endl;
    
    
    RSA* rsa = nullptr;
    BIGNUM* bn = BN_new();
    EVP_PKEY* pkey = nullptr;
    BIO* bio = nullptr;
    
    // 初始化RSA结构
    rsa = RSA_new();
    if (!rsa) {
        std::cerr << "RSA_new failed" << std::endl;
        return 1;
    }

    // 设置公钥指数为65537
    BN_set_word(bn, RSA_F4);
    
    // 生成2048位RSA密钥对
    if (RSA_generate_key_ex(rsa, 2048, bn, nullptr) != 1) {
        std::cerr << "RSA_generate_key_ex failed" << std::endl;
        //goto cleanup;
        return  0;
    }

    // 转换RSA结构为EVP_PKEY
    pkey = EVP_PKEY_new();
    if (!pkey || EVP_PKEY_assign_RSA(pkey, rsa) != 1) {
        std::cerr << "EVP_PKEY assign failed" << std::endl;
        return 0;
    }
    rsa = nullptr; // 所有权已转移给pkey

    // 提取密钥参数
    const BIGNUM *n, *e, *d;
    RSA_get0_key(EVP_PKEY_get0_RSA(pkey), &n, &e, &d);

    // 转换为HEX字符串
    char* n_hex = BN_bn2hex(n);
    char* e_hex = BN_bn2hex(e);
    char* d_hex = BN_bn2hex(d);

    // 输出结果
    std::cout << "Public Key (n):\n" << n_hex << std::endl;
    std::cout << "length:\n" << strlen(n_hex) << std::endl;
    std::cout << "\nPublic Exponent (e):\n" << e_hex << std::endl;
    std::cout << "length:\n" << strlen(n_hex) << std::endl;
    std::cout << "\nPrivate Exponent (d):\n" << d_hex << std::endl;

    // 清理内存
    OPENSSL_free(n_hex);
    OPENSSL_free(e_hex);
    OPENSSL_free(d_hex);


    

    return 0;
}
