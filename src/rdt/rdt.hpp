#ifndef SNAP_PLUGIN_COLLECTOR_RDT_RDT_HPP
#define SNAP_PLUGIN_COLLECTOR_RDT_RDT_HPP

#include <vector>

#include "pqos.h"

#include "snap/plugin.h"
#include "snap/config.h"
#include "snap/metric.h"

namespace rdt {

    class Collector : public Plugin::CollectorInterface {
    public:
        Collector();
        ~Collector();

        const Plugin::ConfigPolicy get_config_policy();
        std::vector<Plugin::Metric> get_metric_types(Plugin::Config cfg);
        void collect_metrics(std::vector<Plugin::Metric> &metrics);
        Plugin::Meta get_plugin_meta();

        std::string name = "rdt";
        int version = 1;

    private:
        Collector(const Collector &that) {};

        std::vector<Plugin::Metric> get_cmt_metrics();

        void setup_cmt_monitoring();
        void poll_metrics();

        std::vector<Plugin::Metric> get_capabilities_metrics();

        const std::vector<Plugin::Metric::NamespaceElement> cmt_capability_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"cmt_capability"}};
        const std::vector<Plugin::Metric::NamespaceElement> mbm_local_monitoring_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"mbm_local_monitoring"}};
        const std::vector<Plugin::Metric::NamespaceElement> l3ca_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"cache_allocation"}};
        const std::vector<Plugin::Metric::NamespaceElement> llc_size_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"llc_size"}};
        const std::vector<Plugin::Metric::NamespaceElement> cache_ways_count_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"cache_ways_count"}};
        const std::vector<Plugin::Metric::NamespaceElement> cache_way_size_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"cache_way_size"}};

        bool is_monitoring_active;
        std::vector<pqos_mon_data*> groups;

        bool cmt_capability;
        bool mbm_local_capability;
        bool l3ca_capability;  // L3 Cache Allocation Capability.

        int core_count;
        int llc_size;
        int cache_ways_count;
        int cache_way_size;
    };

}  // namespace rdt

#endif //SNAP_PLUGIN_COLLECTOR_RDT_RDT_HPP
