//
//  main.cpp
//  BoostRegex
//
//  Created by baojian on 2025/6/26.
//


#include <iostream>
#include <boost/regex.hpp>
#include <map>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>


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

class IpMatcherPcre2 {
private:
    // 存储原始指针，不使用智能指针的删除器
    std::unordered_map<std::string, pcre2_code_8*> regexPatternMap;

public:
    // 析构函数：手动释放所有编译后的正则表达式资源
    ~IpMatcherPcre2() {
        for (auto& entry : regexPatternMap) {
            if (entry.second != nullptr) {
                pcre2_code_free_8(entry.second);
            }
        }
        regexPatternMap.clear();
    }

    // 禁用拷贝构造和赋值运算符（避免资源二次释放）
    IpMatcherPcre2(const IpMatcherPcre2&) = delete;
    IpMatcherPcre2& operator=(const IpMatcherPcre2&) = delete;

    // 允许移动构造和移动赋值
    IpMatcherPcre2(IpMatcherPcre2&&) = default;
    IpMatcherPcre2& operator=(IpMatcherPcre2&&) = default;

    // 构造函数
    IpMatcherPcre2() = default;

    // 添加模式并编译正则表达式
    bool addPattern(std::string&& pattern) {
        return addPatternImpl(std::move(pattern));
    }
    
    bool addPattern(const std::string& pattern) {
        return addPatternImpl(pattern);
    }
    
    bool hasWhiteList() {
        return !regexPatternMap.empty();
    }
    
    // 检查IP是否在白名单中
    bool inWhiteList(std::string&& ip) const {
        return match(ip.data(), ip.size());
    }
    
    bool inWhiteList(const std::string& ip) const {
        return match(ip.data(), ip.size());
    }
    
    bool inWhiteList(std::string_view ip) const {
        return match(ip.data(), ip.size());
    }

private:
    // 实现添加模式的共同逻辑
    bool addPatternImpl(std::string pattern) {
        if (regexPatternMap.find(pattern) != regexPatternMap.end()) {
            return false; // 模式已存在
        }

        int errorCode;
        PCRE2_SIZE errorOffset;
        
 
        pcre2_code_8* re = pcre2_compile_8(
            reinterpret_cast<PCRE2_SPTR8>(pattern.c_str()),
            pattern.length(),
            0, // 选项：默认模式
            &errorCode,
            &errorOffset,
            nullptr
        );
        
        if (!re) {
            PCRE2_UCHAR8 buffer[256];
            pcre2_get_error_message_8(errorCode, buffer, sizeof(buffer));
            std::cout << "pattern error " << pattern << buffer << std::endl;
            return false;
        }
        
        regexPatternMap.emplace(std::move(pattern), re);
        return true;
    }
    
    // 执行匹配的共同逻辑
    bool match(const char* ip, size_t length) const {
        if (regexPatternMap.empty()) {
            return false;
        }
      
        bool result = false;
        // 尝试所有模式进行匹配
        for (const auto& entry : regexPatternMap) {
            pcre2_match_data_8* matchData = pcre2_match_data_create_from_pattern_8(
                entry.second,
                nullptr
            );
            
            if (!matchData) {
                return false;
            }
            
            const pcre2_code_8* re = entry.second;
            
            // 执行匹配
            int matchResult = pcre2_match_8(
                re,
                reinterpret_cast<PCRE2_SPTR8>(ip),
                length,
                0, // 从字符串开头开始匹配
                0, // 匹配选项
                matchData,
                nullptr
            );
            pcre2_match_data_free_8(matchData);
            
            // 匹配成功（返回值大于0表示成功匹配）
            if (matchResult > 0) {
                result = true;
                break;
            }
        }
        return result;
    }
};
    


int main(int argc, const char * argv[]) {
    std::string ip = "192.168.0.1";
    std::string whistList = "192.168.*.*";
    boost::regex pattern(whistList);
   
    std::cout << "match " << boost::regex_match(ip, pattern) << std::endl;
    
    std::cout << "match " << boost::regex_match("127.0.0.1", pattern) << std::endl;
    {
        IpMatcher ipMatcher;
        ipMatcher.addPattern("127.0.0.*");
        ipMatcher.addPattern("192.168.*.*");
        
        std::cout << "match " << ipMatcher.inWhiteList(ip) << std::endl;
        
        std::cout << "match " <<ipMatcher.inWhiteList("127.0.0.1") << std::endl;
        
        //test 127.0.0.1 be convert to 0000:0000:0000:0000:0000:ffff:7f00:0001
        //test 192.168.71.87 be convert to 0000:0000:0000:0000:0000:ffff:c0a8:4757
        
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        bool in = false;
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<10000*200; i++) {
            in = ipMatcher.inWhiteList(ip);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << in << "ipMatcher.inWhiteList used " << used.count() << "ms" << std::endl;
    }
    
    {
        IpMatcherPcre2 ipMatcher;
        ipMatcher.addPattern("127.0.0.*");
        ipMatcher.addPattern("192.168.*.*");
        
        std::cout << "match " << ipMatcher.inWhiteList(ip) << std::endl;
        
        std::string localIp = "127.0.0.1";
        std::cout << "match " <<ipMatcher.inWhiteList(localIp) << std::endl;
        
        //test 127.0.0.1 be convert to 0000:0000:0000:0000:0000:ffff:7f00:0001
        //test 192.168.71.87 be convert to 0000:0000:0000:0000:0000:ffff:c0a8:4757
        
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        bool in = false;
        start = std::chrono::high_resolution_clock::now();
        for(int i=0; i<10000*200; i++) {
            in = ipMatcher.inWhiteList(ip);
        }
        end = std::chrono::high_resolution_clock::now();
        used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << in << "IpMatcherPcre2.inWhiteList used " << used.count() << "ms" << std::endl;
    }
   
    
   
    
    return 0;
}
