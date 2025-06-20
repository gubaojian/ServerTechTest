//
//  main.cpp
//  UTest
//
//  Created by baojian on 2025/6/20.
//

#include <iostream>
#include "is_utf8.h"
#include "simdutf.h"

static bool isValidUtf8(unsigned char *s, size_t length)
{
    for (unsigned char *e = s + length; s != e; ) {
        if (s + 16 <= e) {
            uint64_t tmp[2];
            memcpy(tmp, s, 16);
            if (((tmp[0] & 0x8080808080808080) | (tmp[1] & 0x8080808080808080)) == 0) {
                s += 16;
                continue;
            }
        }

        while (!(*s & 0x80)) {
            if (++s == e) {
                return true;
            }
        }

        if ((s[0] & 0x60) == 0x40) {
            if (s + 1 >= e || (s[1] & 0xc0) != 0x80 || (s[0] & 0xfe) == 0xc0) {
                return false;
            }
            s += 2;
        } else if ((s[0] & 0xf0) == 0xe0) {
            if (s + 2 >= e || (s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80 ||
                    (s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) || (s[0] == 0xed && (s[1] & 0xe0) == 0xa0)) {
                return false;
            }
            s += 3;
        } else if ((s[0] & 0xf8) == 0xf0) {
            if (s + 3 >= e || (s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80 || (s[3] & 0xc0) != 0x80 ||
                    (s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) || (s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4) {
                return false;
            }
            s += 4;
        } else {
            return false;
        }
    }
    return true;
}

/**
 * debug mode
 * uwebsockets faster
 * release mode.
 * uwebsockets isutf8 used 413
 * is_utf8 used 63
 * uwebsockets isutf8 used 6037
 * is_utf8 used 993
 *
 * uwebsockets isutf8 used 874
 * is_utf8 used 143
 * simdutf::validate_utf8 used 142
 *
 * https://github.com/WizzyGeek/Fast-Mask/blob/main/speedup.c
 * https://github.com/fast-pack/MaskedVByte
 * https://github.com/simdutf/
 */

int main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string msg;
    
    for(int i=0; i<512; i++) {
        msg.append("测");
        char ch = i%26 + 'a';
        msg.append(1, ch);
    }
     
    std::cout << msg << std::endl;
    
  
    
    bool isutf8 = false;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<1024*1024; i++) {
        isutf8 = isValidUtf8((unsigned char*)msg.data(), msg.size());
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << isutf8 << std::endl;
    std::cout << "uwebsockets isutf8 used " << used.count() << std::endl;
    
    bool isutf82 = false;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<1024*1024; i++) {
        isutf82 = is_utf8(msg.data(), msg.size());
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << isutf82 << std::endl;
    std::cout << "is_utf8 used " << used.count() << std::endl;
    
    
    bool isutf83 = false;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<1024*1024; i++) {
        isutf83 =  simdutf::validate_utf8(msg.data(), msg.size());
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << isutf83 << std::endl;
    std::cout << "simdutf::validate_utf8 used " << used.count() << std::endl;
    
    return 0;
}
