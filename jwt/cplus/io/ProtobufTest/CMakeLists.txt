cmake_minimum_required(VERSION 3.31)
project(ProtobufTest)

set(CMAKE_CXX_STANDARD 20)



# 确保使用静态库（可选，根据实际情况调整）
set(Protobuf_USE_STATIC_LIBS ON)

# 查找Protobuf（强制使用静态库）
find_package(Protobuf REQUIRED)


include_directories(/usr/local/include)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/homebrew/Cellar/abseil/20240722.1/lib/cmake)


find_package(GTest REQUIRED)

find_package(utf8_range REQUIRED)
find_package(absl REQUIRED)

message(STATUS "Protobuf: ${Protobuf_LIBRARIES} ${absl} ${absl_LIBRARIES} ${utf8_range_LIBRARIES}")

add_executable(ProtobufTest main.cpp protocol_message.pb.h protocol_message.pb.cc)
target_link_libraries(
    ProtobufTest
    ${Protobuf_LIBRARIES}
    absl::base
    absl::strings
    absl::time
    absl::check
    absl::log
    absl::status
    absl::statusor
    absl::flat_hash_map
    absl::flat_hash_set
    absl::node_hash_map
    absl::node_hash_set
    absl::log_severity
    utf8_range::utf8_range
)
