//
//  output.h
//  OutputInputTest
//
//  Created by baojian on 2025/6/10.
//

#ifndef OUTPUT_H
#define OUTPUT_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

class Output {
private:
    uint8_t* buffer;
    size_t position;

public:
    Output(uint8_t* buffer, size_t start)
        : buffer(buffer), position(start) {}

    // 禁止拷贝构造和赋值，避免浅拷贝问题
    Output(const Output&) = delete;
    Output& operator=(const Output&) = delete;

    size_t getPosition() const {
        return position;
    }
    
    size_t getSize() const {
        return position;
    }

    void writeByte(uint8_t value) {
        buffer[position++] = value;
    }

    void writeBytes(const uint8_t* bytes, size_t length) {
        std::memcpy(buffer + position, bytes, length);
        position += length;
    }
    
    void writeStringUtf8(const std::string& str) {
        size_t length = str.length();
        if (length <= 96) {
            writeByte(0);
            writeShortStringUtf8(str);
        } else {
            writeByte(1);
            writeLongStringUtf8(str);
        }
    }
    
    void writeStringUtf8(const char* data, size_t length) {
        if (length <= 96) {
            writeShortStringUtf8(data, length);
        } else {
            writeLongStringUtf8(data, length);
        }
    }

    void writeShortStringUtf8(const std::string& str) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(str.c_str());
        size_t length = str.length();
        if (length > 96) {
            throw std::invalid_argument("short string should be less than 96 bytes");
        }
        writeByte(static_cast<uint8_t>(length));
        writeBytes(bytes, length);
    }
    
    void writeShortStringUtf8(const char* data, size_t length) {
        if (length > 96) {
            throw std::invalid_argument("short string should be less than 96 bytes");
        }
        writeByte(static_cast<uint8_t>(length));
        writeBytes((uint8_t*)data, length);
    }

    void writeLongStringUtf8(const std::string& str) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(str.c_str());
        size_t length = str.length();
        writeInt(static_cast<int32_t>(length));
        writeBytes(bytes, length);
    }
    
    void writeLongStringUtf8(const char* data, size_t length) {
        writeInt(static_cast<int32_t>(length));
        writeBytes((uint8_t*)data, length);
    }

    void writeBinary(const uint8_t* bytes, size_t length) {
        writeInt(static_cast<int32_t>(length));
        writeBytes(bytes, length);
    }

    void writeInt(int32_t value) {
        buffer[position++] = static_cast<uint8_t>(value >> 24);
        buffer[position++] = static_cast<uint8_t>(value >> 16);
        buffer[position++] = static_cast<uint8_t>(value >> 8);
        buffer[position++] = static_cast<uint8_t>(value);
    }
};

#endif // OUTPUT_H
