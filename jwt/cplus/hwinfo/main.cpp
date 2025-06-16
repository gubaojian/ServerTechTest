#include <iostream>

#include "hwinfo/hwinfo.h"
#include "hwinfo/utils/sysctl.h"
#include "hwinfo/utils/unit.h"
#include "machineid/machineid.h"
#include "machineid/include/machineid/machineid.h"

using hwinfo::unit::bytes_to_MiB;
// TIP 要<b>Run</b>代码，请按 <shortcut actionId="Run"/> 或点击装订区域中的 <icon src="AllIcons.Actions.Execute"/> 图标。
int main() {

    std::cout << machineid::machineHash() << std::endl;

    const auto cpus = hwinfo::getAllCPUs();

    for (auto cpu : cpus) {
        std::cout << cpu.vendor() << std::endl;
        std::cout << cpu.modelName() << std::endl;
        std::cout << cpu.numPhysicalCores() << std::endl;
        std::cout << cpu.numLogicalCores() << std::endl;
        std::cout << cpu.maxClockSpeed_MHz() << std::endl;
        std::cout << cpu.regularClockSpeed_MHz() << std::endl;
        std::cout << cpu.L1CacheSize_Bytes() << std::endl;
        std::cout << cpu.L2CacheSize_Bytes() << std::endl;
    }

    hwinfo::OS os;

    std::cout << os.name()  << os.version() << os.kernel() << std::endl;

    std::cout << os.is64bit() << os.isLittleEndian() << os.isBigEndian() << std::endl;

    hwinfo::Memory memory;

    std::cout << memory.total_Bytes() << memory.available_Bytes() << memory.free_Bytes() << std::endl;


    auto networks = hwinfo::getAllNetworks();
    for (auto network : networks) {
        std::cout << network.ip4() << std::endl;
        std::cout << network.ip6() << std::endl;
        std::cout << network.mac() << std::endl;
        std::cout << network.description() << std::endl;
    }

    std::cout << "networks`" << networks.size() << " networks`" << std::endl;

    auto disks = hwinfo::getAllDisks();
    for (auto disk : disks) {
        std::cout << disk.vendor() << std::endl;
        std::cout << disk.model() << std::endl;
    }


    return 0;
    // TIP 请访问 <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a> 查看 CLion 帮助。此外，您还可以从主菜单中选择“帮助 | 学习 IDE 功能”，尝试 CLion 的交互式课次。
}