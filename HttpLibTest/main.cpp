#include <fstream>
#include <iostream>
#include "httplib.h"

using namespace httplib;

static std::string readFile(const std::string& filePath, bool binary = true) {
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

static void writeFile(const std::string& filePath, const std::string& content, bool binary = true) {
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

int main() {
    Server svr;
    svr.Get("/maven/.*", [](const Request &req, Response &res) {

      {
          for (const auto& header : req.headers) {  // 遍历所有头字段
             std::cout << "  " << header.first << ": " << header.second << "\n";
          }
          std::cout << "  " << req.path << "\n";
          if (!req.body.empty()) {
              std::string path = req.path;
                path.insert(0, "test");
                writeFile(path, req.body);
          }
      }

      auto authorization = req.get_header_value("Authorization");
        if (authorization.empty()) {
            res.status = Unauthorized_401;
            res.set_header("WWW-Authenticate", "Basic realm=\"Protected Resource\"");
          std::string str = "401 not found";
            res.set_content(str, "text/plain");
        }

        res.status = StatusCode::NotFound_404;
      std::string str = "404 not found";
        res.set_content(str, "text/plain");
    });

    svr.Put("/maven/.*", [](const Request &req, Response &res) {

      {
          for (const auto& header : req.headers) {  // 遍历所有头字段
             std::cout << "  " << header.first << ": " << header.second << "\n";
          }
          std::cout << " PUT " << req.path << "\n";
          if (!req.body.empty()) {
              std::string path = req.path;
                path.insert(0, "test");
                writeFile(path, req.body);
          }
      }


      std::string str = "Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!";
      res.set_content(str, "text/plain");
    });

    svr.Post("/maven/.*", [](const Request &req, Response &res) {

      {
          for (const auto& header : req.headers) {  // 遍历所有头字段
             std::cout << "  " << header.first << ": " << header.second << "\n";
          }
          std::cout << " POST " << req.path << "\n";
          if (!req.body.empty()) {
              std::string path = req.path;
                path.insert(0, "test");
                writeFile(path, req.body);
          }
      }


      std::string str = "Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!";
      res.set_content(str, "text/plain");
    });

    svr.Put("/bigfile/.*", [](const Request &req, Response &res, const ContentReader &content_reader) {
         for (const auto& header : req.headers) {  // 遍历所有头字段
             std::cout << "  " << header.first << ": " << header.second << "\n";
          }
          std::cout << " bigfile PUT" << req.path
           << " is big file " << req.is_multipart_form_data()
           << "req.body.length()" << req.body.length()
          << "\n";

        if (req.is_multipart_form_data()) {
         // NOTE: `content_reader` is blocking until every form data field is read
         // This approach allows streaming processing of large files
         std::vector<FormData> items;
         content_reader(
           [&](const FormData &item) {
             items.push_back(item);
             return true;
           },
           [&](const char *data, size_t data_length) {
             items.back().content.append(data, data_length);
             std::cout << "form item size " << items.back().content.size() << std::endl;
             return true;
           });

         // Process the received items
         for (const auto& item : items) {
           if (item.filename.empty()) {
             // Text field
             std::cout << "Field: " << item.name << " = " << item.content << std::endl;
           } else {
             // File
             std::cout << "File: " << item.name << " (" << item.filename << ") - "
                       << item.content.size() << " bytes" << std::endl;
           }
         }
       } else {
         std::string body;
         content_reader([&](const char *data, size_t data_length) {
           body.append(data, data_length);
             std::cout << "body.append " << body.size() << "data block " << data_length << std::endl;
           return true;
         });
       }
    });

    svr.Put("/bigmemoryfile/.*", [](const Request &req, Response &res) {
        for (const auto& header : req.headers) {  // 遍历所有头字段
               std::cout << "  " << header.first << ": " << header.second << "\n";
            }
            std::cout << " bigfile PUT" << req.path
             << " is big file " << req.is_multipart_form_data()
             << "req.body.length()" << req.body.length()
            << "\n";
    });

    svr.listen("0.0.0.0", 8080);
}
