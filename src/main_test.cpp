#include <iostream>
#include <sstream>

#include "snap/plugin.h"
#include "rdt/rdt.hpp"

std::string print_ns(Plugin::Metric metric) {
    std::ostringstream stream;
    for (auto iter = metric.ns().begin(); iter != metric.ns().end(); iter++) {
        stream << "/" << (*iter).value;
    }
    return stream.str();
}

int main() {
    try {
        rdt::Collector rdt;

        std::vector<Plugin::Metric> metrics;
        rdt.collect_metrics(metrics);

        for (auto iter = metrics.begin(); iter != metrics.end(); iter++) {
            std::cout << "namespace: " << print_ns(*iter) << " | int data: " << (*iter).get_int_data() << std::endl;
        }

        return 0;
    } catch (const char* what) {
        fprintf(stderr, "Cannot launch RDT Collector: %s\n", what);
        return -1;
    }
}

