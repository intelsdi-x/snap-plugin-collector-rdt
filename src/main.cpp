#include <iostream>

#include "snap/plugin.h"
#include "rdt/rdt.hpp"

int main() {
    int exit_code = -1;
    auto pqos = new PQOS();
    rdt::Collector *rdt;
    try
    {
        rdt = new rdt::Collector(new PQOS());
        start_collector(rdt, rdt->get_plugin_meta());
        exit_code = 0;
    } catch (const char* what) {
        fprintf(stderr, "Cannot launch RDT Collector: %s\n", what);
    }
    delete(rdt);
    delete(pqos);
    return exit_code;
}
