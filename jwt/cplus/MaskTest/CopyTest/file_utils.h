#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <fstream>
#include <stdexcept>

class FileUtils {
public:
    /**
     * 读取文件全部内容并返回为字符串
     * @param filePath 文件路径
     * @param binary 是否以二进制模式读取（默认为否）
     * @return 文件内容字符串
     * @throws std::runtime_error 当文件打开失败或读取错误时
     */
    static std::string readFile(const std::string& filePath, bool binary = false) {
        std::ios_base::openmode mode = std::ios::in;
        if (binary) {
            mode |= std::ios::binary;
        }
        
        std::ifstream file(filePath, mode);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }
        
        // 使用文件流的缓冲区直接构造字符串
        std::string content;
        file.seekg(0, std::ios::end);
        content.reserve(file.tellg());
        file.seekg(0, std::ios::beg);
        
        content.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
        
        if (file.bad()) {
            throw std::runtime_error("Error reading file: " + filePath);
        }
        
        return content;
    }
    
    static void writeFile(const std::string& filePath, const std::string& content, bool binary = false) {
        std::ios_base::openmode mode = std::ios::out | std::ios::trunc; // 默认覆盖写入
        if (binary) {
            mode |= std::ios::binary;
        }
        
        std::ofstream file(filePath, mode);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }
        
        // 使用流迭代器写入内容
        std::copy(content.begin(), content.end(), std::ostreambuf_iterator<char>(file));
        
        if (file.bad()) {
            throw std::runtime_error("Error writing to file: " + filePath);
        }
        
        // 可选：显式刷新缓冲区
        file.flush();
    }
};

#endif // FILE_UTILS_H    
