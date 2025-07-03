#include <iostream>

#include "protocol_message.pb.h"

// TIP 要<b>Run</b>代码，请按 <shortcut actionId="Run"/> 或点击装订区域中的 <icon src="AllIcons.Actions.Execute"/> 图标。
int main() {

    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::string message(1024, 'a');
    std::string wsgId = "wsgId3332";
    std::string connId = "2235823572375_3332_38888";

    start = std::chrono::high_resolution_clock::now();
    std::string out;
    for(int i=0; i<10000*200; i++) {
        wsg::gateway::protocol::PackMessage pack;
        pack.mutable_appid()->assign(wsgId);
        pack.mutable_connid()->assign(connId);
        pack.mutable_msg()->assign(message);
        pack.SerializeToString(&out);
        out.clear();
    }
    end = std::chrono::high_resolution_clock::now();
    used = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "pack binary used" << used.count() << "ms" << std::endl;

    {
        wsg::gateway::protocol::PackMessage pack;
        pack.mutable_appid()->assign(wsgId);
        pack.mutable_connid()->assign(connId);
        pack.mutable_msg()->assign(message);
        pack.SerializeToString(&out);
        std::cout << "pack binary length" << out.length() << std::endl;
    }



    return 0;
    // TIP 请访问 <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a> 查看 CLion 帮助。此外，您还可以从主菜单中选择“帮助 | 学习 IDE 功能”，尝试 CLion 的交互式课次。
}