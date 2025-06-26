//
//  main.cpp
//  BoostRegex
//
//  Created by baojian on 2025/6/26.
//


#include <iostream>
#include <boost/regex.hpp>
#include <map>


class IpMatcher {
    
public:
    void addPattern(std::string&& pattern) {
        auto it = regexPatternMap.find(pattern);
        if (it != regexPatternMap.end()) {
            return;
        }
        regexPatternMap[pattern] = std::make_shared<boost::regex>(pattern);
    }
    void addPattern(std::string& pattern) {
        auto it = regexPatternMap.find(pattern);
        if (it != regexPatternMap.end()) {
            return;
        }
        regexPatternMap[pattern] = std::make_shared<boost::regex>(pattern);
    }
    
    bool inWhiteList(std::string&& ip) {
        auto it = regexPatternMap.begin();
        while (it != regexPatternMap.end()) {
            auto& pattern = it->second;
            if (boost::regex_match(ip, *pattern)) {
                return true;
            }
            it++;
        }
        return false;
    }
    
    bool inWhiteList(std::string& ip) {
        auto it = regexPatternMap.begin();
        while (it != regexPatternMap.end()) {
            auto& pattern = it->second;
            if (boost::regex_match(ip, *pattern)) {
                return true;
            }
            it++;
        }
        return false;
    }
    
    bool inWhiteList(std::string_view& ip) {
        auto it = regexPatternMap.begin();
        while (it != regexPatternMap.end()) {
            auto& pattern = it->second;
            if (boost::regex_match(ip.data(), ip.data() + ip.size(), *pattern)) {
                return true;
            }
            it++;
        }
        return false;
    }
    
private:
    std::unordered_map<std::string, std::shared_ptr<boost::regex>> regexPatternMap;
};


int main(int argc, const char * argv[]) {
    std::string ip = "192.168.0.1";
    std::string whistList = "192.168.*.*";
    boost::regex pattern(whistList);
   
    std::cout << "match " << boost::regex_match(ip, pattern) << std::endl;
    
    std::cout << "match " << boost::regex_match("127.0.0.1", pattern) << std::endl;
    
    IpMatcher ipMatcher;
    ipMatcher.addPattern("127.0.0.*");
    ipMatcher.addPattern("192.168.*.*");
    
    std::cout << "match " << ipMatcher.inWhiteList(ip) << std::endl;
    
    std::cout << "match " <<ipMatcher.inWhiteList("127.0.0.1") << std::endl;
    
    //test 127.0.0.1 be convert to 0000:0000:0000:0000:0000:ffff:7f00:0001
    //test 192.168.71.87 be convert to 0000:0000:0000:0000:0000:ffff:c0a8:4757
   
    
    return 0;
}
