#include <iostream>

#include "snap/plugin.h"
#include "rdt/rdt.hpp"

int main() {
    RDT rdt;
    std::cout << rdt.name << " " << rdt.version << std::endl;
    start_collector(&rdt, rdt.get_plugin_meta());
    return 0;
}