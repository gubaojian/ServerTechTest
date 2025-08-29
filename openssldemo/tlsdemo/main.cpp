#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bn.h>

#define BUF_SIZE 1024
#define PLAINTEXT "Hello, SSL BIO in-memory demo! This is test data."
#define HANDSHAKE_TIMEOUT 30  // 总超时次数（每次50ms，共1.5秒）

// 错误处理
void handle_ssl_error(const char *msg) {
    fprintf(stderr, "[ERROR] %s: \n", msg);
    ERR_print_errors_fp(stderr);
    ERR_get_error();
    ERR_clear_error();
    exit(EXIT_FAILURE);
}

// 创建临时证书
X509* create_temp_cert(EVP_PKEY *pkey) {
    X509 *cert = X509_new();
    if (!cert) return NULL;

    X509_set_version(cert, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);
    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), 31536000);

    X509_NAME *name = X509_get_issuer_name(cert);
    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char*)"CN", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char*)"TestOrg", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"TestServer", -1, -1, 0);
    X509_set_subject_name(cert, name);

    X509_set_pubkey(cert, pkey);
    if (!X509_sign(cert, pkey, EVP_sha256())) {
        X509_free(cert);
        return NULL;
    }
    return cert;
}

int main() {
    // 1. 初始化OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    OpenSSL_add_all_ciphers();

    // 2. 创建SSL上下文（TLSv1.2）
    const SSL_METHOD *method = TLSv1_2_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) handle_ssl_error("SSL_CTX_new failed");

    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!EXPORT:!SSLv2:!RC4+MD5:!LOW");

    // 3. 生成密钥和证书
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!pctx) handle_ssl_error("EVP_PKEY_CTX_new_id failed");

    if (EVP_PKEY_keygen_init(pctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0 ||
        EVP_PKEY_keygen(pctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        handle_ssl_error("RSA key generation failed");
    }
    EVP_PKEY_CTX_free(pctx);

    X509 *cert = create_temp_cert(pkey);
    if (!cert || SSL_CTX_use_certificate(ctx, cert) <= 0 ||
        SSL_CTX_use_PrivateKey(ctx, pkey) <= 0) {
        handle_ssl_error("Load cert/key failed");
    }

    // 4. 创建内存BIO对（单线程核心：双向关联的内存缓冲区）
    BIO *server_mem = NULL, *client_mem = NULL;
    if (!BIO_new_bio_pair(&server_mem, BUF_SIZE, &client_mem, BUF_SIZE) ||
        !server_mem || !client_mem) {
        handle_ssl_error("BIO_new_bio_pair failed");
    }

    // 5. 创建SSL BIO链（服务端+客户端）
    BIO *server_ssl_bio = BIO_new_ssl(ctx, 0);  // 服务端模式
    BIO *client_ssl_bio = BIO_new_ssl(ctx, 1);  // 客户端模式
    if (!server_ssl_bio || !client_ssl_bio ||
        !BIO_push(server_ssl_bio, server_mem) ||
        !BIO_push(client_ssl_bio, client_mem)) {
        handle_ssl_error("SSL BIO chain failed");
    }

    // 6. 单线程握手核心逻辑：交替处理客户端和服务端事件
    printf("Starting single-thread SSL handshake...\n");
    int client_done = 0, server_done = 0;  // 双方握手完成标志
    int timeout_count = 0;
    char buf[BUF_SIZE];

    // 统一循环处理双方握手（单线程关键）
    while (!(client_done && server_done)) {
        // 处理客户端握手
        if (!client_done) {
            int ret = BIO_do_connect(client_ssl_bio);
            if (ret > 0) {
                client_done = 1;  // 客户端握手完成
                printf("Client handshake done\n");
            } else if (ret <= 0 && !BIO_should_retry(client_ssl_bio)) {
                handle_ssl_error("Client handshake failed");
            }
            // 客户端需要重试时，强制服务端读取数据（推动交互）
            else if (BIO_should_retry(client_ssl_bio)) {
                printf("Client waiting, server reading...\n");
                int server_ret = BIO_read(server_ssl_bio, buf, BUF_SIZE);
                if (server_ret < 0 && !BIO_should_retry(server_ssl_bio)) {
                    handle_ssl_error("Server read failed (client retry)");
                }
            }
        }

        // 处理服务端握手（与客户端交替执行）
        if (!server_done) {
            int ret = BIO_do_accept(server_ssl_bio);
            if (ret > 0) {
                server_done = 1;  // 服务端握手完成
                printf("Server handshake done\n");
            } else if (ret <= 0 && !BIO_should_retry(server_ssl_bio)) {
                handle_ssl_error("Server handshake failed");
            }
            // 服务端需要重试时，强制客户端读取数据（推动交互）
            else if (BIO_should_retry(server_ssl_bio)) {
                printf("Server waiting, client reading...\n");
                int client_ret = BIO_read(client_ssl_bio, buf, BUF_SIZE);
                if (client_ret < 0 && !BIO_should_retry(client_ssl_bio)) {
                    handle_ssl_error("Client read failed (server retry)");
                }
            }
        }

        // 超时控制
        if (++timeout_count > HANDSHAKE_TIMEOUT) {
            handle_ssl_error("Handshake timed out (single-thread)");
        }
        usleep(50000);  // 50ms延迟，给数据处理时间
    }

    printf("SSL handshake completed (single-thread success)\n");

    // 7. 服务端发送数据
    int ret = BIO_write(server_ssl_bio, PLAINTEXT, strlen(PLAINTEXT));
    if (ret <= 0) handle_ssl_error("Server write failed");
    printf("Server sent %d bytes: %s\n", ret, PLAINTEXT);

    // 8. 客户端接收数据
    char decrypted[BUF_SIZE] = {0};
    ret = BIO_read(client_ssl_bio, decrypted, BUF_SIZE - 1);
    if (ret <= 0) handle_ssl_error("Client read failed");

    ret = BIO_read(client_ssl_bio, decrypted, BUF_SIZE - 1);
    printf("Client read IO_should_retry %d %ld\n", BIO_should_retry(client_ssl_bio), ERR_get_error());

    if (ret <= 0) handle_ssl_error("Client read failed retry");

    // 9. 输出结果
    printf("\n=== Result ===\n");
    printf("Original: %s\n", PLAINTEXT);
    printf("Decrypted: %s\n", decrypted);
    printf("Length: %d bytes\n", ret);

    // 10. 释放资源
    BIO_free_all(client_ssl_bio);
    BIO_free_all(server_ssl_bio);
    X509_free(cert);
    EVP_PKEY_free(pkey);
    SSL_CTX_free(ctx);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();

    return 0;
}
