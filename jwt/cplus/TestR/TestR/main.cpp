//
//  main.cpp
//  TestR
//
//  Created by efurture on 2025/5/28.
//

#include <iostream>
#include <string>
#include <sstream>

std::string getHtmlContent() {
    std::stringstream htmlStream;
    htmlStream << R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>示例页面</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>欢迎来到示例页面</h1>
        <p>这是一个使用stringstream转换的HTML字符串示例。</p>
        <p>你可以在这里添加更多内容...</p>
    </div>
</body>
</html>)";
    return htmlStream.str();
}

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    std::cout << getHtmlContent() << std::endl;
    std::string hello =  R"(<!DOCTYPE html>
<html lang="zh-CN">)";
    return 0;
}
