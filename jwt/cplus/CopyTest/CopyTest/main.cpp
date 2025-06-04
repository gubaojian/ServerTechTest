//
//  main.cpp
//  CopyTest
//
//  Created by efurture on 2025/6/4.
//

#include <iostream>
#include <string>
#include <memory>

/**
 * https://bugs.webkit.org/show_bug.cgi?id=79880
 * https://bugs.webkit.org/attachment.cgi?id=130519&action=diff
 *  https://chromium.googlesource.com/chromium/src.git/+/500dbe3cfc18c490cc466abef162896302284814%5E%21/#F0
 *  https://bugs.webkit.org/attachment.cgi?id=130519&action=diff
 *
 *  websocket mask speed up。
 *   https://github.com/WebKit/WebKit/blob/e8e08c2a3be413bf639b32bd5940717402b6f1b6/Source/WebCore/Modules/websockets/WebSocketFrame.cpp
 *
 *    https://chromium.googlesource.com/chromium/src/+/d95453989b7c1/net/websockets/websocket_frame.cc
 *
 *   https://bugs.webkit.org/attachment.cgi?id=130519&action=diff
 *
 *     https://chromium.googlesource.com/chromium/src.git/+/500dbe3cfc18c490cc466abef162896302284814
 *
 *
 *   simd64 used 61
 *  simd32 used 103
 *  used 668
 *
 */


/// Four byte conversion union
union uint32_converter {
    uint32_t i;
    uint8_t c[4];
};

/// Eight byte conversion union
union uint64_converter {
    uint64_t i;
    uint8_t  c[8];
};


typedef uint32_converter masking_key_type;


/// Byte by byte mask/unmask
/**
 * Iterator based byte by byte masking and unmasking for WebSocket payloads.
 * Performs masking in place using the supplied key offset by the supplied
 * offset number of bytes.
 *
 * This function is simple and can be done in place on input with arbitrary
 * lengths and does not vary based on machine word size. It is slow.
 *
 * @param b Beginning iterator to start masking
 *
 * @param e Ending iterator to end masking
 *
 * @param o Beginning iterator to store masked results
 *
 * @param key 32 bit key to mask with.
 *
 * @param key_offset offset value to start masking at.
 */
template <typename input_iter, typename output_iter>
void byte_mask(input_iter first, input_iter last, output_iter result,
    masking_key_type const & key, size_t key_offset = 0)
{
    size_t key_index = key_offset%4;
    while (first != last) {
        *result = *first ^ key.c[key_index++];
        key_index %= 4;
        ++result;
        ++first;
    }
}

/// Copy and mask/unmask in one operation
/**
 * Reads input from one string and writes unmasked output to another.
 *
 * @param [in] i The input string.
 * @param [out] o The output string.
 * @param [in] key The masking key to use for masking/unmasking
 */
void masked_copy (std::string const & i, std::string & o,
    masking_key_type key)
{
    byte_mask(i.begin(),i.end(),o.begin(),key);
}


void masked_copy_simd64 (std::string const & i, std::string & o,
    masking_key_type key)
{
    uint64_converter u64Key;
    std::memcpy(u64Key.c, key.c, 4);
    std::memcpy(u64Key.c + 4, key.c, 4);
    size_t length = i.size()/8;
    uint64_t* u64I = (uint64_t*)i.data();
    uint64_t* u64O = (uint64_t*)o.data();
    for(int i=0; i<length; i++){
        u64O[i] = u64I[i] ^ u64Key.i;
    }
    size_t remain = i.length()%8;
    if (remain > 0) {
        size_t offset = i.size() - remain;
        uint8_t* rI = (uint8_t*)(i.data() + offset);
        uint8_t* rO = (uint8_t*)(o.data() + offset);
        for (int i=0; i<remain; i++) {
            rO[i] = rI[i] ^ key.c[i%4];
        }
    }
}

void masked_copy_simd32(std::string const & i, std::string & o,
    masking_key_type key)
{
    uint32_converter u32Key;
    std::memcpy(u32Key.c, key.c, 4);
    size_t length = i.size()/4;
    uint32_t* u32I = (uint32_t*)i.data();
    uint32_t* u32O = (uint32_t*)o.data();
    for(int i=0; i<length; i++){
        u32O[i] = u32I[i] ^ u32Key.i;
    }
    size_t remain = i.length()%4;
    if (remain > 0) {
        size_t offset = i.size() - remain;
        uint8_t* rI = (uint8_t*)(i.data() + offset);
        uint8_t* rO = (uint8_t*)(o.data() + offset);
        for (int i=0; i<remain; i++) {
            rO[i] = rI[i] ^ key.c[i%4];
        }
    }
}

void byte_mask2(char* first, char* last, char* result,
               uint8_t* mask, size_t key_offset)
{
    size_t key_index = key_offset%4;
    while (first != last) {
        *result = *first ^ mask[key_index++];
        key_index %= 4;
        ++result;
        ++first;
    }
}

/**
 * 64simd used 84
 * used 1032
 */
int main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    {
        std::string in(1025, 'a');
        std::string out;
        std::string out2;
        std::string out3;
        out.resize(in.size());
        out2.resize(in.size());
        out3.resize(in.size());
       
        masking_key_type key;
        key.i = 100;
        masked_copy_simd32(in, out3, key);
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            masked_copy_simd64(in, out2, key);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "simd64 used " << used.count() << std::endl;
        
     
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            masked_copy_simd32(in, out3, key);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "simd32 used " << used.count() << std::endl;
        
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            masked_copy(in, out, key);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "used " << used.count() << std::endl;
        

        std::cout << "eq " << (out == out2) << std::endl;
        std::cout << "eq " << (out == out3) << std::endl;
    }
    
    
    
    

    
    
    
    
    return 0;
}
