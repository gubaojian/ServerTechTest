#include <iostream>
#include <thread>

#include "simdjson.h"


 std::string json1024 = R"(
    {
    "user": {
        "id": 123456,
        "name": "John Doe",
        "email": "john.doe@example.com",
        "is_active": true,
        "age": 32,
        "location": {
            "city": "New York",
            "country": "USA",
            "coordinates": {
                "latitude": 40.7128,
                "longitude": -74.0060
            }
        },
        "preferences": {
            "theme": "dark",
            "notifications": {
                "email": true,
                "push": false,
                "sms": false
            },
            "language": "en-US"
        }
    },
    "orders": [
        {
            "order_id": "ORD-789012",
            "product": "Laptop Pro",
            "quantity": 1,
            "price": 1299.99,
            "status": "shipped",
            "shipping_address": {
                "street": "123 Main St",
                "city": "New York",
                "zip": "10001"
            }
        },
        {
            "order_id": "ORD-345678",
            "product": "Wireless Headphones",
            "quantity": 2,
            "price": 149.99,
            "status": "delivered",
            "shipping_address": {
                "street": "456 Elm St",
                "city": "New York",
                "zip": "10002"
            }
        }
    ],
    "settings": {
        "timezone": "America/New_York",
        "date_format": "YYYY-MM-DD",
        "currency": "USD",
        "units": {
            "length": "inches",
            "weight": "pounds"
        },
        "security": {
            "2fa_enabled": true,
            "last_login": "2023-06-15T08:30:00Z",
            "devices": [
                {
                    "name": "MacBook Pro",
                    "os": "macOS",
                    "last_used": "2023-06-15T08:30:00Z"
                },
                {
                    "name": "iPhone",
                    "os": "iOS",
                    "last_used": "2023-06-14T18:45:00Z"
                }
            ]
        }
    }
}
     )";

std::string json512 = R"(
    {
    "app": {
        "name": "Signal Desktop",
        "version": "6.2.0",
        "build": "20230610",
        "platform": "macOS",
        "architecture": "x64",
        "features": {
            "end_to_end_encryption": true,
            "group_chats": true,
            "file_sharing": true,
            "reactions": true
        }
    },
    "user": {
        "id": "u12345",
        "username": "alice_signal",
        "display_name": "Alice Johnson",
        "profile": {
            "bio": "Software Engineer | Privacy Advocate",
            "avatar": "https://picsum.photos/200/200?random=1",
            "status": "online"
        },
        "contacts": [
            {
                "id": "u67890",
                "name": "Bob Smith",
                "last_seen": "2023-06-14T20:15:30Z"
            },
            {
                "id": "u54321",
                "name": "Charlie Brown",
                "last_seen": "2023-06-15T09:45:10Z"
            }
        ]
    },
    "preferences": {
        "theme": "dark",
        "notifications": {
            "muted_chats": ["u98765"],
            "sound": "default",
            "vibrate": true
        },
        "privacy": {
            "read_receipts": true,
            "typing_indicators": true,
            "profile_visibility": "contacts"
        }
    }
}
)";

std::string json128 = R"(


{
    "id": "evt-12345",
    "type": "message.received",
    "timestamp": 1686837900,
    "data": {
        "sender": "bob@example.com",
        "recipient": "alice@example.com",
        "content": "Hello, Alice! This is a test message.",
        "attachments": [],
        "metadata": {
            "encrypted": true,
            "read": false
        }
    }
}

)";

// TIP 要<b>Run</b>代码，请按 <shortcut actionId="Run"/> 或点击装订区域中的 <icon src="AllIcons.Actions.Execute"/> 图标。
int main() {

    {
        simdjson::ondemand::parser parser;
        simdjson::padded_string json(json1024);
        simdjson::ondemand::document tweets = parser.iterate(json);
        tweets["user"]["email"];
    }

    {
        simdjson::ondemand::parser parser;
        char* buffer = new char[1024*128];
        std::memcpy(buffer, json1024.data(), json1024.size());
        std::memset(buffer + json1024.size(), 0, simdjson::SIMDJSON_PADDING);
        simdjson::padded_string_view json(buffer, json1024.size() + simdjson::SIMDJSON_PADDING);
        simdjson::ondemand::document tweets = parser.iterate(json);
        tweets["user"]["email"];
        delete[] buffer;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
     {
         auto start = std::chrono::high_resolution_clock::now();
         simdjson::ondemand::parser parser;
         for (int i=0; i<1000*1000*2; i++) {
             {
                 simdjson::padded_string json(json1024);
                 simdjson::ondemand::document tweets = parser.iterate(json);
                 tweets["user"]["email"];
             }
             {
                 simdjson::padded_string json(json128);
                 simdjson::ondemand::document tweets = parser.iterate(json);
                 tweets["id"];
             }

             {
                 simdjson::padded_string json(json512);
                 simdjson::ondemand::document tweets = parser.iterate(json);
                 tweets["app"]["platform"];
             }

             {
                 simdjson::padded_string json(json128);
                 simdjson::ondemand::document tweets = parser.iterate(json);
                 tweets["id"];
             }
         }
         auto end = std::chrono::high_resolution_clock::now();
         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
         std::cout << "used " << duration  << "ms" << std::endl;
     }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    {
         char* buffer = new char[1024*128];
         auto start = std::chrono::high_resolution_clock::now();
         simdjson::ondemand::parser parser;
         for (int i=0; i<1000*1000*2; i++) {
             {
                 std::memcpy(buffer, json1024.data(), json1024.size());
                 std::memset(buffer + json1024.size(), 0, simdjson::SIMDJSON_PADDING);
                 simdjson::padded_string_view json(buffer, json1024.size() + simdjson::SIMDJSON_PADDING);
                 simdjson::ondemand::document tweets = parser.iterate(json);
                 tweets["user"]["email"];
             }
             {
                 std::memcpy(buffer, json128.data(), json128.size());
                 std::memset(buffer + json128.size(), 0, simdjson::SIMDJSON_PADDING);
                 simdjson::padded_string_view json(buffer, json128.size() + simdjson::SIMDJSON_PADDING);
                 simdjson::ondemand::document tweets = parser.iterate(json);
                 tweets["id"];
             }

             {
                 std::memcpy(buffer, json512.data(), json512.size());
                 std::memset(buffer + json512.size(), 0, simdjson::SIMDJSON_PADDING);
                 simdjson::padded_string_view json(buffer, json512.size() + simdjson::SIMDJSON_PADDING);
                 simdjson::ondemand::document tweets = parser.iterate(json);
                 tweets["app"]["platform"];
             }

             {
                 std::memcpy(buffer, json128.data(), json128.size());
                 std::memset(buffer + json128.size(), 0, simdjson::SIMDJSON_PADDING);
                 simdjson::padded_string_view json(buffer, json128.size() + simdjson::SIMDJSON_PADDING);
                 simdjson::ondemand::document tweets = parser.iterate(json);
                 tweets["id"];
             }

         }
         auto end = std::chrono::high_resolution_clock::now();
         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
         std::cout << "string_view used " << duration  << "ms" << std::endl;
         delete[] buffer;
    }



    return 0;
    // TIP 请访问 <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a> 查看 CLion 帮助。此外，您还可以从主菜单中选择“帮助 | 学习 IDE 功能”，尝试 CLion 的交互式课次。
}