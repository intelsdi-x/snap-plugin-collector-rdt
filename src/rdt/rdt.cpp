#include "rdt.hpp"


const Plugin::ConfigPolicy RDT::get_config_policy() {
    return Plugin::ConfigPolicy{};
}

std::vector<Plugin::Metric> RDT::get_metric_types(Plugin::Config cfg) {
    return std::vector<Plugin::Metric>{};
}
void RDT::collect_metrics(std::vector<Plugin::Metric> &metrics) {
    return;
}

Plugin::Meta RDT::get_plugin_meta() {
    return Plugin::Meta(Plugin::Collector, this->name, this->version);
}