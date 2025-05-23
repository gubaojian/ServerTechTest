//
//  main.cpp
//  jwttest
//
//  Created by efurture on 2025/5/15.
//

#include <jwt-cpp/jwt.h>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <iostream>

int main(int argc, const char * argv[]) {
    std::string const token = "eyJhbGciOiJIUzI1NiJ9.eyJjb25uSWQiOiI3ZDIyOTZjZi1hMzFlLTQ5YjctYWQwZi1hOGRhODlhOWI4NjkiLCJpYXQiOjE3NDc4ODEwODl9.bY72ZsXHB4KjnsJTFyyXV0ygZznmAuf4H51MSYGqjZU";
    std::string secret = "i8zIHoTLy2t4uMIztIUi3vA129xYVKAE";
       auto decoded = jwt::decode(token);
    
    std::cout << decoded.get_payload_json().find("connId")->second << std::endl;

    for(auto& e : decoded.get_header_json())
        std::cout << e.first << " = " << e.second << '\n';
    
    for(auto& e : decoded.get_payload_json())
           std::cout << e.first << " = " << e.second << '\n';
    
    std::cout << "alg " << decoded.get_algorithm() << std::endl;
    
    auto playload = decoded.get_payload_json();
    auto header = decoded.get_header_json();
    auto algIt = header.find("alg");
    if (algIt == header.end()) {
        return 0;
    }
    auto alg = algIt->second.to_str();
    std::transform(alg.begin(), alg.end(), alg.begin(), ::toupper);
    auto verifier = jwt::verify();
    if ("HS256" == alg) {
        verifier = verifier.allow_algorithm(jwt::algorithm::hs256{secret});
    } else {
        return 0;
    }
        
    for(auto it = playload.begin(); it != playload.end(); it++) {
        if (it->first == "exp" || it->first == "iat") {
            continue;
        }
        verifier = verifier.with_claim(it->first, jwt::claim(it->second));
    }

    
    std::error_code ec;
    verifier.verify(decoded, ec);
    
    std::cout <<  ec << std::endl;
    
    auto timeSinceEpo = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto timePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(std::chrono::milliseconds(timeSinceEpo));
    auto time =  std::chrono::system_clock::to_time_t(timePoint);
    //std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&time);
    
      // std::cout.imbue(std::locale("ru_RU.utf8"));
       std::cout << "ru_RU: " << std::put_time(&tm, "%c %Z") << '\n';
    
       //std::cout.imbue(std::locale("ja_JP.utf8"));
    std::cout << "ja_JP: " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S %Z") << '\n';
    
    return 0;
}
