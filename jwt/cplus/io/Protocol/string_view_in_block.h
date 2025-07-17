//
// Created by baojian on 25-7-17.
//

#ifndef STRING_BLOCK_SUB_VIEW_H
#define STRING_BLOCK_SUB_VIEW_H
#include <string>
#include <memory>
#include <string_view>

class StringViewInBlock {
public:
    explicit StringViewInBlock(const std::shared_ptr<std::string>& fullBlock) {
        block = fullBlock;
        messageView = *fullBlock;
    }
    explicit StringViewInBlock(const std::shared_ptr<std::string>& block, const std::string_view& messageViewInBlock) {
        this->block = block;
        this->messageView = messageViewInBlock;
    }
public:
    std::string_view messageView;
private:
    std::shared_ptr<std::string> block;
};

#endif //STRING_BLOCK_SUB_VIEW_H
