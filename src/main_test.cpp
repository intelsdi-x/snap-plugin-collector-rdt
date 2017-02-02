#include <iostream>

#include "snap/plugin.h"
#include "rdt/rdt.hpp"

int main() {
    try {
        rdt::Collector rdt;

//        rdt.get_metric_types();

        std::vector<Plugin::Metric> metrics;
        rdt.collect_metrics(metrics);

        for (auto iter = metrics.begin(); iter != metrics.end(); iter++) {
//            std::cout << *iter << std::endl;
        }

        return 0;
    } catch (const char* what) {
        fprintf(stderr, "Cannot launch RDT Collector: %s\n", what);
        return -1;
    }
}
