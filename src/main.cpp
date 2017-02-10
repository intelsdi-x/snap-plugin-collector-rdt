#include <iostream>

#include "snap/plugin.h"
#include "rdt/rdt.hpp"

int main() {
    try {
        rdt::Collector rdt;
        start_collector(&rdt, rdt.get_plugin_meta());
        return 0;
    } catch (const char* what) {
        fprintf(stderr, "Cannot launch RDT Collector: %s\n", what);
        return -1;
    }
}
