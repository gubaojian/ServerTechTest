//
//  main.cpp
//  jwttest
//
//  Created by efurture on 2025/5/15.
//

#include <jwt-cpp/jwt.h>
#include <iostream>

int main(int argc, const char * argv[]) {
    std::string const token = "eyJhbGciOiJIUzI1NiJ9.eyJjb25uSWQiOiJoZWxsbyB3b3JsZCIsImlhdCI6MTc0NzI3NTg3OCwiZXhwIjoxNzQ3MzYyMjc4fQ.NBUB8TAlBsL8wYxs7No6AsfiozrYlvHTr252wPAri7g";
    std::string secret = "testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest";
       auto decoded = jwt::decode(token);
    
    std::cout << decoded.get_payload_json().find("connId")->second << std::endl;

    for(auto& e : decoded.get_header_json())
        std::cout << e.first << " = " << e.second << '\n';
    
    for(auto& e : decoded.get_payload_json())
           std::cout << e.first << " = " << e.second << '\n';
    
    std::cout << "alg " << decoded.get_algorithm() << std::endl;
    
    auto verifier = jwt::verify()
        .with_claim("connId", jwt::claim(std::string("hello world")))
        .allow_algorithm(jwt::algorithm::hs256{secret});
    
    std::error_code ec;
    verifier.verify(decoded, ec);
    
    std::cout <<  ec << std::endl;
    
    return 0;
}
