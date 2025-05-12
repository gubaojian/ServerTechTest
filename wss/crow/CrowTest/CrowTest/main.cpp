//
//  main.cpp
//  CrowTest
//
//  Created by efurture on 2025/5/10.
//

#include <iostream>

#include <crow.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    crow::SimpleApp app;

    CROW_ROUTE(app, "/").methods("POST"_method)([](){
        return "Hello world";
    });

    app.port(18080).multithreaded().run();
    return 0;
}
