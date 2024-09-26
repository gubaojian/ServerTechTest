//
//  share_ptr_bug.cc
//  cplus
//
//  Created by efurture on 2024/9/26.
//

#include <iostream>
#include <map>
#include <string>
#include <memory>

int share_ptr_test_main(int argc, const char * argv[]) {
    std::unordered_map<std::string, std::string> mapStrs;
    std::string hello1 = "hello";
    std::string hello2 = "hello";
    std::string world = "world";
    mapStrs[hello1] = "world";
    mapStrs[hello2] = "world";
    
    std::cout << "map str size " << mapStrs.size() << " capcity " << mapStrs.bucket_count() << std::endl;
    for(const auto& pair : mapStrs) {
        std::cout << "key: " << pair.first  << " value: " << pair.second << std::endl;
    }
    
    
    //share_ptr作为key容易引起的bug
    std::unordered_map<std::shared_ptr<std::string>, std::string> mapSPtr;
    std::string helloworld = "hello world";
    std::shared_ptr<std::string> helloPtr1 = std::make_shared<std::string>("hello world");
    std::shared_ptr<std::string> helloPtr2 = std::make_shared<std::string>("hello world");
    
    mapSPtr[helloPtr1] = world;
    mapSPtr[helloPtr2] = world;
    
    std::cout << "map shareptr size " << mapSPtr.size() << " capcity " << mapSPtr.bucket_count() << std::endl;
    for(const auto& pair : mapSPtr) {
        std::cout << "key: " << pair.first  << " value: " << pair.second << std::endl;
    }
    
    std::cout << "share_ptr different with std::string as map key" << std::endl;
    return 0;
}
