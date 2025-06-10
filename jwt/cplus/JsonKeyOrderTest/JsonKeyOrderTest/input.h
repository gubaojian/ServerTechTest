//
//  input.h
//  OutputInputTest
//
//  Created by baojian on 2025/6/10.
//
#ifndef INPUT_H
#define INPUT_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string_view>

class Input {
private:
    const uint8_t* buffer;
    size_t position;
    size_t start;

public:
    Input(const uint8_t* buffer, size_t start)
        : buffer(buffer), position(start), start(start) {}

    // 禁止拷贝构造和赋值，避免指针浅拷贝问题
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    uint8_t readByte() {
        return buffer[position++];
    }

    int32_t readInt() {
        int32_t value =
            (static_cast<int32_t>(buffer[position]) << 24) |
            (static_cast<int32_t>(buffer[position + 1]) << 16) |
            (static_cast<int32_t>(buffer[position + 2]) << 8) |
            static_cast<int32_t>(buffer[position + 3]);
        position += 4;
        return value;
    }

    std::string_view readShortStringUtf8() {
        uint8_t length = readByte();
        std::string_view str(reinterpret_cast<const char*>(buffer + position), length);
        position += length;
        return str;
    }

    std::string_view readLongStringUtf8() {
        int32_t length = readInt();
        std::string_view str(reinterpret_cast<const char*>(buffer + position), length);
        position += length;
        return str;
    }

    std::string_view readBinary() {
        int32_t length = readInt();
        std::string_view binary(reinterpret_cast<const char*>(buffer + position), length);
        position += length;
        return binary;
    }

    size_t getPosition() const {
        return position;
    }
};

#endif // INPUT_H
