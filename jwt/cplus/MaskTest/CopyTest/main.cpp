//
//  main.cpp
//  CopyTest
//
//  Created by efurture on 2025/6/4.
//

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include "WebSocketProtocol.h"
#include "file_utils.h"



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
 * simd128 used 26
 simd64 used 50
 simd32 used 69
 normal used 508
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


struct buffer {
    buffer(char const * b, size_t l) : buf(b),len(l) {}

    char const * buf;
    size_t len;
};

unsigned int messages = 0;
unsigned int test_count = 0;

struct Impl {
    static bool refusePayloadLength(uint64_t length, uWS::WebSocketState<true> *wState, void *s) {

        /* We need a limit */
        if (length > 16000) {
            return true;
        }

        /* Return ok */
        return false;
    }

    static bool setCompressed(uWS::WebSocketState<true> *wState, void *s) {
        /* We support it */
        return true;
    }

    static void forceClose(uWS::WebSocketState<true> *wState, void *s, std::string_view reason = {}) {

    }

    static bool handleFragment(char *data, size_t length, unsigned int remainingBytes, int opCode, bool fin, uWS::WebSocketState<true> *webSocketState, void *s) {
        
        std::string_view str (data, length);
        
        std::cout <<"parse " << str << std::endl;
        if (remainingBytes <= 0) {
            if (opCode == uWS::TEXT) {
                if (!uWS::protocol::isValidUtf8((unsigned char *)data, length)) {
                    /* Return break */
                    //return true;
                }
            } else if (opCode == uWS::CLOSE) {
                uWS::protocol::parseClosePayload((char *)data, length);
            }

            messages += 1;
            
            if (length > 0) {
                test_count += data[rand()%length];
            }
        }
        

        /* Return ok */
        return false;
    }
};


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
    uint8_t mask[8] = {key.c[0], key.c[1], key.c[2], key.c[3], key.c[0], key.c[1], key.c[2], key.c[3]};
    uint64_t maskInt;
    memcpy(&maskInt, mask, 8);
    
    size_t length64 = i.size()/8;
    uint64_t* u64I = (uint64_t*)i.data();
    uint64_t* u64O = (uint64_t*)o.data();
    for(size_t m=0; m<length64; m++){
        u64O[m] = u64I[m] ^ maskInt;
    }
    size_t remain = i.size()%8;
    if (remain > 0) {
        size_t offset = i.size() - remain;
        char* rI = (char*)(i.data() + offset);
        char* rO = (char*)(o.data() + offset);
        for (size_t i=0; i<remain; i++) {
            rO[i] = rI[i] ^ key.c[i % 4];
        }
    }
}

void masked_copy_simd128(std::string const & i, std::string & o,
    masking_key_type key)
{
    uint8_t mask[8] = {key.c[0], key.c[1], key.c[2], key.c[3], key.c[0], key.c[1], key.c[2], key.c[3]};
    uint64_t maskInt;
    memcpy(&maskInt, mask, 8);
    
    size_t length64 = i.size()/8;
    uint64_t* u64I = (uint64_t*)i.data();
    uint64_t* u64O = (uint64_t*)o.data();
    size_t loop2Length = (length64/2)*2;
    for(size_t i=0; i<loop2Length; i+=2){
        size_t two = i + 1;
        u64O[i] = u64I[i] ^ maskInt;
        u64O[two] = u64I[two] ^ maskInt;
    }
    if(loop2Length < length64){
        u64O[loop2Length] = u64I[loop2Length] ^ maskInt;
    }
    size_t remain = i.length() % 8;
    if (remain > 0) {
        size_t offset = i.size() - remain;
        char* rI = (char*)(i.data() + offset);
        char* rO = (char*)(o.data() + offset);
        for (size_t i=0; i<remain; i++) {
            rO[i] = rI[i] ^ key.c[i % 4];
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


void testMaskBench() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    {
        std::string in = FileUtils::readFile("/Users/baojian/code/ServerTechTest/jwt/cplus/io/UTest/UTest/test_2.json", true);
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
        for(int i=0; i<1000*1000*2; i++) {
            masked_copy_simd128(in, out2, key);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "simd128 used " << used.count() << std::endl;
        
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000*1000*2; i++) {
            masked_copy_simd64(in, out2, key);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "simd64 used " << used.count() << std::endl;
        
        uWS::WebSocketProtocol<true, Impl> protocol;
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000*1000*2; i++) {
            protocol.unmaskImpreciseCopyMask<14>((char*)in.data() + 20, in.size() - 20);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "WebSocketProtocol unmaskImpreciseCopyMask used " << used.count() << std::endl;
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1000*1000*2; i++) {
            protocol.unmaskPreciseCopyMask<14>((char*)in.data() + 20, in.size() - 20);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "WebSocketProtocol unmaskPreciseMask used " << used.count() << std::endl;
     
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            masked_copy_simd32(in, out3, key);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "simd32 used " << used.count() << std::endl;
        
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            //masked_copy(in, out, key);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "normal used " << used.count() << std::endl;
        

        std::cout << "eq " << (out == out2) << std::endl;
        std::cout << "eq " << (out == out3) << std::endl;
    }
    
}


void testCopyBench() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    {
        std::string in(1024, 'a');
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            std::string ap;
            ap.reserve(1024);
            ap.append(in);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "append used " << used.count() << std::endl;
        
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            std::string ap = in;
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "eq used " << used.count() << std::endl;
        
     
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            std::string cp;
            cp.resize(1024);
            std::memcpy((char*)cp.data(), (char*)in.data(), in.size());
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "cp used " << used.count() << std::endl;
    }
    
}



void testCopyReSize() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    {
        std::string in(1024, 'a');
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            std::string ap;
            ap.resize(1024);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "resize used " << used.count() << std::endl;
        
        
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<1024*1024; i++) {
            std::string c(1024, '\n');
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "cp used " << used.count() << std::endl;
        
    }
    
}




void testProtocolBench() {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::string binaryFrame = FileUtils::readFile("/Users/baojian/code/ServerTechTest/jwt/cplus/MaskTest/CopyTest/binary.dat", true);

    uWS::WebSocketProtocol<true, Impl> protocol;
    uWS::WebSocketState<true> state;
    std::vector<std::shared_ptr<std::string>> datas;
    for(int i=0; i<1024*1024; i++) {
        datas.push_back(std::make_shared<std::string>(binaryFrame));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    /**
    uWS::switch_fast = true;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<1024*1024; i++) {
        std::shared_ptr<std::string> frame = datas.at(i);
        protocol.consume(frame->data(), frame->length(), &state, nullptr);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "fast prepare count " << 1024*1024 << std::endl;
    std::cout << "fast message count " << messages << std::endl;
    std::cout << "fast protocol used " << used.count() << std::endl;*/
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    uWS::switch_fast = false;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<1024*1024; i++) {
        std::shared_ptr<std::string> frame = datas.at(i);
        protocol.consume(frame->data(), frame->length(), &state, nullptr);
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "prepare count " << 1024*1024 << std::endl;
    std::cout << "message count " << messages << std::endl;
    std::cout << "protocol used " << used.count() << std::endl;
    
    std::cout << test_count << std::endl;
}

void testProtocolPartParse() {
    std::string framedata = FileUtils::readFile("/Users/baojian/code/ServerTechTest/jwt/cplus/MaskTest/CopyTest/one_frame.dat", true);
    
    std::string two;
    two.append(framedata);
    two.append(framedata);

    uWS::WebSocketProtocol<true, Impl> protocol;
    uWS::WebSocketState<true> state;
    
    char* src = (char*)framedata.data();
    
    std::string first_read = framedata;
    std::string second_read = framedata;
    std::string third_read = framedata;
    //part parse
    protocol.consume(first_read.data(), 19, &state, nullptr);
    protocol.consume(second_read.data() + 19, 23, &state, nullptr);
    protocol.consume(third_read.data() + 19 + 23, framedata.size() - 19 - 23, &state, nullptr);
    
    
    //parse two message
    protocol.consume(two.data(), two.size(), &state, nullptr);
    
    std::cout << "parse two message" << std::endl;
    std::cout << state.state.wantsHead << std::endl;

}


/**
 * 64simd used 84
 * used 1032
 *
 * boost asio速度更快：
 *   https://github.com/boostorg/beast/issues/1699
 */
int main(int argc, const char * argv[]) {
    
    testMaskBench();
    
    //testCopyBench();
    
    //testCopyReSize();
    
    //testProtocolBench();
    
    testProtocolPartParse();
    
    
    
    
    return 0;
}
