#ifndef SNAP_PLUGIN_COLLECTOR_RDT_RDT_H
#define SNAP_PLUGIN_COLLECTOR_RDT_RDT_H

#include "snap/plugin.h"
#include "snap/config.h"
#include "snap/metric.h"


class RDT : public Plugin::CollectorInterface{
public:
    const Plugin::ConfigPolicy get_config_policy();
    std::vector<Plugin::Metric> get_metric_types(Plugin::Config cfg);
    void collect_metrics(std::vector<Plugin::Metric> &metrics);
    Plugin::Meta get_plugin_meta();

    std::string name = "rdt";
    int version = 1;

};


#endif //SNAP_PLUGIN_COLLECTOR_RDT_RDT_H
