clang++ json_test.cc yyjson-master/src/yyjson.c wson.c  -O2 -std=c++20

#clang++ json_test.cc yyjson-master/src/yyjson.c wson.c  -g -std=c++20



clang++ json_test.cc yyjson-master/src/yyjson.c wson.c message.pb.cc  -O2 -std=c++20 -lprotobuf


 ./bin/protoc message.proto -I=./ --cpp_out=./ --java_out=./

 protoc /Users/efurture/Downloads/wson-master/message.proto --cpp_out=/Users/efurture/Downloads/wson-master -I=/Users/efurture/Downloads/wson-master
