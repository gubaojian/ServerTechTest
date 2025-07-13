//
// Created by efurture on 25-7-13.
//

#ifndef IO_OUTPUT_INPUT_H
#define IO_OUTPUT_INPUT_H
#include <string_view>
#include <string>
#include <cstdint>
#include <cstring>
#include <iostream>
#include "config/app_config.h"

namespace wsg {
    namespace gateway {

        class Output {
        private:
            char* buffer;
            size_t position;  // 使用size_t替代int

        public:
            Output(char* buffer, size_t initialPosition = 0)
                : buffer(buffer), position(initialPosition) {}

            std::string_view toBytes() const {
                return {buffer, position};
            }

            void writeByte(char value) {
                buffer[position++] = value;
            }

            void writeBytes(const char* data, size_t length) {
                memcpy(buffer + position, data, length);
                position += length;
            }

            void writeVarInt2(size_t value) {
                uint8_t* uint8_buffer = ((uint8_t*)(buffer));
                if (value <= 0x7F) {
                    uint8_buffer[position++] = (uint8_t)value;
                } else if (value <= 0x3FFF) {
                    uint8_buffer[position++] = (uint8_t)((value & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(value >> 7);
                } else if (value <= 0x1FFFFF) {
                    uint8_buffer[position++] = (uint8_t)((value & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(((value >> 7) & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(value >> 14);
                } else if (value <= 0xFFFFFFF) {
                    uint8_buffer[position++] = (uint8_t)((value & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(((value >> 7) & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(((value >> 14) & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(value >> 21);
                } else {
                    uint8_buffer[position++] = (uint8_t)((value & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(((value >> 7) & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(((value >> 14) & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(((value >> 21) & 0x7F) | 0x80);
                    uint8_buffer[position++] = (uint8_t)(value >> 28);
                }
            }

            void writeVarInt(size_t value) {
                uint8_t* uint8_buffer = ((uint8_t*)(buffer +  position));
                if (value <= 0x7F) {
                    uint8_buffer[0] = (uint8_t)value;
                    position +=1;
                } else if (value <= 0x3FFF) {
                    uint8_buffer[0] = (uint8_t)((value & 0x7F) | 0x80);
                    uint8_buffer[1] = (uint8_t)(value >> 7);
                    position+=2;
                } else if (value <= 0x1FFFFF) {
                    uint8_buffer[0] = (uint8_t)((value & 0x7F) | 0x80);
                    uint8_buffer[1] = (uint8_t)(((value >> 7) & 0x7F) | 0x80);
                    uint8_buffer[2] = (uint8_t)(value >> 14);
                    position+=3;
                } else if (value <= 0xFFFFFFF) {
                    uint8_buffer[0] = (uint8_t)((value & 0x7F) | 0x80);
                    uint8_buffer[1] = (uint8_t)(((value >> 7) & 0x7F) | 0x80);
                    uint8_buffer[2] = (uint8_t)(((value >> 14) & 0x7F) | 0x80);
                    uint8_buffer[3] = (uint8_t)(value >> 21);
                    position+=4;
                } else {
                    uint8_buffer[0] = (uint8_t)((value & 0x7F) | 0x80);
                    uint8_buffer[1] = (uint8_t)(((value >> 7) & 0x7F) | 0x80);
                    uint8_buffer[2] = (uint8_t)(((value >> 14) & 0x7F) | 0x80);
                    uint8_buffer[3] = (uint8_t)(((value >> 21) & 0x7F) | 0x80);
                    uint8_buffer[4] = (uint8_t)(value >> 28);
                    position+=5;
                }
            }

            void writeBinary(const std::string_view& binary) {
                writeBinary(binary.data(), binary.size());
            }

            void writeBinary(const char* data, size_t length) {
                writeVarInt(static_cast<int32_t>(length));  // 显式转换为int32_t
                memcpy(buffer + position, data, length);
                position += length;
            }

            void writeInt(int32_t val) {
                //should use uint8 to keep same across platform. char is signed or unsigned not same on platform。
                uint8_t* uint8_buffer = ((uint8_t*)buffer);
                uint8_buffer[position + 3] = (uint8_t)val;
                uint8_buffer[position + 2] = (uint8_t)(val >> 8);
                uint8_buffer[position + 1] = (uint8_t)(val >> 16);
                uint8_buffer[position] = (uint8_t)(val >> 24);
                position += 4;
            }

            size_t getPosition() const { return position; }  // 新增：获取当前位置
        };



        class Input {
        private:
            const char* buffer;
            size_t position;
            size_t bufferSize;
            bool hasError;

        public:
            Input(const char* buffer,  size_t pos, size_t bufferSize)
                : buffer(buffer), hasError(false), position(pos), bufferSize(bufferSize) {}

            const char* getBuffer() const { return buffer; }
            size_t getPosition() const { return position; }
            void setPosition(size_t pos) { position = pos; }

            char readByte() {
                if (position >= bufferSize) {
                    setErrorFormatFlag();
                    return 0;
                }
                return buffer[position++];
            }

            int32_t readInt() {
                if (position + 4 > bufferSize) {
                    setErrorFormatFlag();
                    return 0;
                }
                const int32_t number =
                    (static_cast<uint8_t>(buffer[position + 3])) +
                    (static_cast<uint8_t>(buffer[position + 2]) << 8) +
                    (static_cast<uint8_t>(buffer[position + 1]) << 16) +
                    (static_cast<uint8_t>(buffer[position]) << 24);
                position += 4;
                return number;
            }

            void skip(size_t length) {
                position += length;
            }

            int32_t readVarInt() {
                auto bts = (const int8_t*)(buffer);
                if (bts[position] >= 0) {
                    return static_cast<uint8_t>(bts[position++]);
                }
                if ((position + 1) < bufferSize && bts[position + 1] >= 0) {
                    // avoid multiple unsequenced modifications
                    int32_t value =  (static_cast<uint8_t>(bts[position]) & 0x7F) |
                           (static_cast<uint8_t>(bts[position + 1]) << 7);
                    position += 2;
                    return value;
                }
                if ((position + 2) < bufferSize && bts[position + 2] >= 0) {
                     int32_t value =  (static_cast<uint8_t>(bts[position]) & 0x7F) |
                           (static_cast<uint8_t>(bts[position + 1]) & 0x7F) << 7 |
                           (static_cast<uint8_t>(bts[position + 2]) << 14);
                    position += 3;
                    return value;
                }
                if ((position + 3) < bufferSize && bts[position + 3] >= 0) {
                    int32_t value = (static_cast<uint8_t>(bts[position]) & 0x7F) |
                           (static_cast<uint8_t>(bts[position + 1]) & 0x7F) << 7 |
                           (static_cast<uint8_t>(bts[position + 2]) & 0x7F) << 14 |
                           (static_cast<uint8_t>(bts[position + 3]) << 21);
                    position += 4;
                    return value;
                }
                if ((position + 4) < bufferSize && bts[position + 4] >= 0) {
                    if ((static_cast<uint8_t>(bts[position + 4]) & 0x7F) > 0x0F) {
                        setErrorFormatFlag();
                        return 0;
                    }
                     int32_t value = (static_cast<uint8_t>(bts[position]) & 0x7F) |
                           (static_cast<uint8_t>(bts[position + 1]) & 0x7F) << 7 |
                           (static_cast<uint8_t>(bts[position + 2]) & 0x7F) << 14 |
                           (static_cast<uint8_t>(bts[position + 3]) & 0x7F) << 21 |
                           (static_cast<uint8_t>(bts[position + 4]) << 28);
                    position += 5;
                    return value;
                }
                setErrorFormatFlag();
                return 0;
            }

            std::string_view readBinary() {
                int32_t length = readVarInt();
                if (length < 0 || position + static_cast<size_t>(length) > bufferSize) {
                    setErrorFormatFlag();
                    return {buffer, 0};
                }
                if (position + length > bufferSize) {
                    setErrorFormatFlag();
                    return {buffer, 0};
                }
                std::string_view view(buffer + position, length);
                position += length;
                return view;
            }

            void skipBinary() {
                int32_t length = readVarInt();
                if (length < 0) {
                    setErrorFormatFlag();
                    return;
                }
                position = std::min(position + length, bufferSize);
            }

            void setErrorFormatFlag() {
                if (wsgDebug) {
                    std::string_view data(buffer, bufferSize);
                    std::cout << data << " error format at" << position << std::endl;
                }
                hasError = true;
                position = bufferSize;
            }

            bool hasNext() const { return position < bufferSize; }
        };



    }
}

#endif //IO_OUTPUT_INPUT_H
