//
//  main.cpp
//  JsonKeyOrderTest
//
//  Created by efurture on 2025/6/9.
//

#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include "simdjson/simdjson.h"
#include "output.h"
#include "input.h"

/**
 * normal data used 331ms
 * wsgId ws_685208380980
 * custom sort data used 448ms
 * wsgId2 ws_685208380980
 * wsgId3 ws_685208380980
 * custom sort data with hand parse used 36ms
 *
 *normal data used 493ms
 custom sort data used 453ms
 custom sort data with hand parse used 20ms
 *
 *
 *normal data used 339ms
 custom sort data used 323ms
 custom sort data with hand parse used 19ms
 binary data with hand parse used 7ms
 *
 */
int main(int argc, const char * argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string str1 = R"({"wsgId":"ws_685208380980","connId":"0c43b07c-783e-49bf-8773-c02dd6cf31da","action":"textMsg","msg":"RVn8HEKBF8rvauQV3ehDx218W20poYSeifPdYTHsOgnslEcD5Elx3k1ea6zMvbUICX1HO6MaVDGXhIbyn43tiy9GbyPw1VAsNRAzIwRvbLOgYmR7jVtSSPuBtjSvNqCYUSAhdmiQ7GrMcLAQCwMrcj6eBFhcsE7qdFrswtl5GJFYmJZFOcnHJ3O1q5WM661M1i4DzGuFDI4c5wedRH1NZDwxXkAXTDAJUTt2LgB7ImJUkhXihKr5fHb4MBIJPy1XEZ4eOFLcJToo0Ti1TOtjc9xEcbkpmTj0FwtyKfzPLdM8IDayIFGFIeH7tYl50AVmj56qMqDyeq3NK5TOJHpytlEDYavARkaL86SnV9RDJ7zWm2pi2ZEIDUsqzlMkvQ81XGAwM3OsPbk6ueJ3KJgo5KN2s22xBr98bdjsWFaqUMT4afr2xUh0JOXjjN68HMMxNCD9nsm1EstlDRZrRmrU1XPcbauoXHAYRmx26n5MzdfjMAB3kvmBrjqq1ndEICWdNGypbu4Sb6BN3TmwZAOwpCGyYCoIdqaZIe2cL8Bqd5ye1y9O0PIueU7aE3ME3mCIEA8h5QXosfN1aYbtF4MoCFCdVRsKMIY5RTGEdzrbaRClRRXXoS2OqunxXQ6FRVy2DlSgXkcM3Uz7QBXatokqOvdRgkBVWQExAExIuzqZ6q8QrRwdWeljCqLTpzE9rXj1o5p9gjRJhk6PHkbgfF4g5vmGUD2sOSfPp2cHg38FS6YXfRPXkKwIx2EpNBlNLyWja4Df1IwZJdM7sNfZuqdpaUcPTON1EmoL2CxTi7uuxFnICMGmBVYyhJgjZrDIUkjay7rNv8gwTRi5wwhw58csnin6K8iEP53V79zi0h2H9pnqUb19UEz2ug92Qvj5tb3103rjmD9GNcEOcznbFIqKbh24m1J5i2bKnlBOclQbzugUkPjYEGo5cDCwLQWw729VxC3JA3vqjSIJrZSXC8qJqJy97U62bFoNZjJruqa7YK5MfQwvoj6NorUbMzZxVcIo"}
    )";
    std::string str2 = R"({"action":"textMsg","msg":"RVn8HEKBF8rvauQV3ehDx218W20poYSeifPdYTHsOgnslEcD5Elx3k1ea6zMvbUICX1HO6MaVDGXhIbyn43tiy9GbyPw1VAsNRAzIwRvbLOgYmR7jVtSSPuBtjSvNqCYUSAhdmiQ7GrMcLAQCwMrcj6eBFhcsE7qdFrswtl5GJFYmJZFOcnHJ3O1q5WM661M1i4DzGuFDI4c5wedRH1NZDwxXkAXTDAJUTt2LgB7ImJUkhXihKr5fHb4MBIJPy1XEZ4eOFLcJToo0Ti1TOtjc9xEcbkpmTj0FwtyKfzPLdM8IDayIFGFIeH7tYl50AVmj56qMqDyeq3NK5TOJHpytlEDYavARkaL86SnV9RDJ7zWm2pi2ZEIDUsqzlMkvQ81XGAwM3OsPbk6ueJ3KJgo5KN2s22xBr98bdjsWFaqUMT4afr2xUh0JOXjjN68HMMxNCD9nsm1EstlDRZrRmrU1XPcbauoXHAYRmx26n5MzdfjMAB3kvmBrjqq1ndEICWdNGypbu4Sb6BN3TmwZAOwpCGyYCoIdqaZIe2cL8Bqd5ye1y9O0PIueU7aE3ME3mCIEA8h5QXosfN1aYbtF4MoCFCdVRsKMIY5RTGEdzrbaRClRRXXoS2OqunxXQ6FRVy2DlSgXkcM3Uz7QBXatokqOvdRgkBVWQExAExIuzqZ6q8QrRwdWeljCqLTpzE9rXj1o5p9gjRJhk6PHkbgfF4g5vmGUD2sOSfPp2cHg38FS6YXfRPXkKwIx2EpNBlNLyWja4Df1IwZJdM7sNfZuqdpaUcPTON1EmoL2CxTi7uuxFnICMGmBVYyhJgjZrDIUkjay7rNv8gwTRi5wwhw58csnin6K8iEP53V79zi0h2H9pnqUb19UEz2ug92Qvj5tb3103rjmD9GNcEOcznbFIqKbh24m1J5i2bKnlBOclQbzugUkPjYEGo5cDCwLQWw729VxC3JA3vqjSIJrZSXC8qJqJy97U62bFoNZjJruqa7YK5MfQwvoj6NorUbMzZxVcIo","connId":"0c43b07c-783e-49bf-8773-c02dd6cf31da",
        "wsgId":"ws_685208380980"}
    )";
    simdjson::ondemand::parser parser;
    std::string wsgId1;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++){
        simdjson::padded_string json = simdjson::padded_string(str2);
        simdjson::ondemand::document doc;
        auto error = parser.iterate(json).get(doc);
        if (error) {
            std::cout << " send wrong format message" << std::endl;
            break;
        }
        auto wsgId = doc["wsgId"].get_string();
        if (wsgId.error()) {
            std::cout << " send wrong format message, missing wsgId" << std::endl;
            break;
        }
        
        wsgId1 = std::string(wsgId.value().data(), wsgId.value().size());
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "normal data used " << used.count() << "ms" << std::endl;
    std::cout << "wsgId " << wsgId1 << std::endl;
  
  
    std::string wsgId2;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++){
        simdjson::padded_string json = simdjson::padded_string(str1);
        simdjson::ondemand::document doc;
        auto error = parser.iterate(json).get(doc);
        if (error) {
            std::cout << " send wrong format message" << std::endl;
            break;
        }
        auto wsgId = doc["wsgId"].get_string();
        if (wsgId.error()) {
            std::cout << " send wrong format message, missing wsgId" << std::endl;
            break;
        }
        
        wsgId2 = std::string(wsgId.value().data(), wsgId.value().size());
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "custom sort data used " << used.count() << "ms" << std::endl;
    std::cout << "wsgId2 " << wsgId2 << std::endl;
    
    std::string wsgId3;
    start = std::chrono::high_resolution_clock::now();
    constexpr std::string prefix = "{\"wsgId\":\"";
    if (str1.starts_with(prefix)) {
        auto pos = str1.find('"', prefix.size() + 1);
        if (pos >= 0) {
            wsgId3 = str1.substr(prefix.size(), pos - prefix.size());
        }
    }
    std::cout << "wsgId3 " <<  wsgId3 << std::endl;
    std::string_view header(str1.data(), 128);
    std::cout << header << std::endl;

    for(int i=0; i<10000*200; i++){
        if (str1.starts_with(prefix)) {
            auto pos = str1.find('"', prefix.size() + 1);
            if (pos > prefix.size()) {
                wsgId3 = str1.substr(prefix.size(), pos - prefix.size());
            }
        } else {
            simdjson::padded_string json = simdjson::padded_string(str1);
            simdjson::ondemand::document doc;
            auto error = parser.iterate(json).get(doc);
            if (error) {
                std::cout << " send wrong format message" << std::endl;
                break;
            }
            auto wsgId = doc["wsgId"].get_string();
            if (wsgId.error()) {
                std::cout << " send wrong format message, missing wsgId" << std::endl;
                break;
            }
            wsgId3 = std::string(wsgId.value().data(), wsgId.value().size());
        }
      
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "custom sort data with hand parse used " << used.count() << "ms" << std::endl;
    std::cout << "wsgId2 " << wsgId2 << std::endl;
    
    
    char buffer[2048];
    Output output((uint8_t*)buffer, 0);
   
    
    output.writeShortStringUtf8("ws_685208380980");
    output.writeShortStringUtf8("textMsg");
    output.writeShortStringUtf8("0c43b07c-783e-49bf-8773-c02dd6cf31da");
    output.writeLongStringUtf8("RVn8HEKBF8rvauQV3ehDx218W20poYSeifPdYTHsOgnslEcD5Elx3k1ea6zMvbUICX1HO6MaVDGXhIbyn43tiy9GbyPw1VAsNRAzIwRvbLOgYmR7jVtSSPuBtjSvNqCYUSAhdmiQ7GrMcLAQCwMrcj6eBFhcsE7qdFrswtl5GJFYmJZFOcnHJ3O1q5WM661M1i4DzGuFDI4c5wedRH1NZDwxXkAXTDAJUTt2LgB7ImJUkhXihKr5fHb4MBIJPy1XEZ4eOFLcJToo0Ti1TOtjc9xEcbkpmTj0FwtyKfzPLdM8IDayIFGFIeH7tYl50AVmj56qMqDyeq3NK5TOJHpytlEDYavARkaL86SnV9RDJ7zWm2pi2ZEIDUsqzlMkvQ81XGAwM3OsPbk6ueJ3KJgo5KN2s22xBr98bdjsWFaqUMT4afr2xUh0JOXjjN68HMMxNCD9nsm1EstlDRZrRmrU1XPcbauoXHAYRmx26n5MzdfjMAB3kvmBrjqq1ndEICWdNGypbu4Sb6BN3TmwZAOwpCGyYCoIdqaZIe2cL8Bqd5ye1y9O0PIueU7aE3ME3mCIEA8h5QXosfN1aYbtF4MoCFCdVRsKMIY5RTGEdzrbaRClRRXXoS2OqunxXQ6FRVy2DlSgXkcM3Uz7QBXatokqOvdRgkBVWQExAExIuzqZ6q8QrRwdWeljCqLTpzE9rXj1o5p9gjRJhk6PHkbgfF4g5vmGUD2sOSfPp2cHg38FS6YXfRPXkKwIx2EpNBlNLyWja4Df1IwZJdM7sNfZuqdpaUcPTON1EmoL2CxTi7uuxFnICMGmBVYyhJgjZrDIUkjay7rNv8gwTRi5wwhw58csnin6K8iEP53V79zi0h2H9pnqUb19UEz2ug92Qvj5tb3103rjmD9GNcEOcznbFIqKbh24m1J5i2bKnlBOclQbzugUkPjYEGo5cDCwLQWw729VxC3JA3vqjSIJrZSXC8qJqJy97U62bFoNZjJruqa7YK5MfQwvoj6NorUbMzZxVcIo");
    
    std::string wsgId4;
    
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<10000*200; i++) {
        Input input((uint8_t*)buffer, 0);
        wsgId4 = input.readShortStringUtf8();
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "binary data with hand parse used " << used.count() << "ms" << std::endl;
    std::cout << "wsgId4 " << wsgId4 << std::endl;


    return 0;
}
