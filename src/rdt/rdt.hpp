#ifndef SNAP_PLUGIN_COLLECTOR_RDT_RDT_HPP
#define SNAP_PLUGIN_COLLECTOR_RDT_RDT_HPP

#include <vector>

#include "pqos.h"

#include "snap/plugin.h"
#include "snap/config.h"
#include "snap/metric.h"

class PQOSInterface
{
public:
    virtual ~PQOSInterface() {}
    virtual int pqos_init(const pqos_config *config) = 0;
    virtual int pqos_cap_get(const struct pqos_cap **cap, const struct pqos_cpuinfo **cpu) = 0;
    virtual int pqos_fini() = 0;
    virtual int pqos_mon_start(const unsigned num_cores, const unsigned *cores, const enum pqos_mon_event event, void *context, struct pqos_mon_data *group) = 0;
    virtual int pqos_mon_reset() = 0;
    virtual int pqos_mon_stop(struct pqos_mon_data *group) = 0;
    virtual int pqos_mon_poll(struct pqos_mon_data **groups, const unsigned num_groups) = 0;
};

class PQOS : public PQOSInterface
{
public:
    int pqos_init(const pqos_config *config);
    int pqos_cap_get(const struct pqos_cap **cap, const struct pqos_cpuinfo **cpu);
    int pqos_fini();
    int pqos_mon_start(const unsigned num_cores, const unsigned *cores, const enum pqos_mon_event event, void *context, struct pqos_mon_data *group);
    int pqos_mon_reset();
    int pqos_mon_stop(struct pqos_mon_data *group);
    int pqos_mon_poll(struct pqos_mon_data **groups, const unsigned num_groups);
};

namespace rdt
{
    const std::vector<Plugin::Metric::NamespaceElement> cmt_capability_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"cmt_capability"}};
    const std::vector<Plugin::Metric::NamespaceElement> mbm_local_monitoring_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"mbm_local_monitoring"}};
    const std::vector<Plugin::Metric::NamespaceElement> l3ca_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"cache_allocation"}};
    const std::vector<Plugin::Metric::NamespaceElement> llc_size_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"llc_size"}};
    const std::vector<Plugin::Metric::NamespaceElement> cache_ways_count_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"cache_ways_count"}};
    const std::vector<Plugin::Metric::NamespaceElement> cache_way_size_ns = {{"intel"}, {"rdt"}, {"capabilities"}, {"cache_way_size"}};

    class Collector : public Plugin::CollectorInterface
    {
    public:
        Collector(PQOSInterface *pqos);
        ~Collector();

        const Plugin::ConfigPolicy get_config_policy();
        std::vector<Plugin::Metric> get_metric_types(Plugin::Config cfg);
        void collect_metrics(std::vector<Plugin::Metric> &metrics);
        Plugin::Meta get_plugin_meta();

        std::string name = "rdt";
        int version = 1;

    private:
        Collector(const Collector &that){};

        std::vector<Plugin::Metric> get_cmt_metrics();

        void setup_cmt_monitoring();
        void poll_metrics();

        std::vector<Plugin::Metric> get_capabilities_metrics();

        bool is_monitoring_active;
        std::vector<pqos_mon_data *> groups;

        bool cmt_capability;
        bool mbm_local_capability;
        bool l3ca_capability; // L3 Cache Allocation Capability.

        int core_count;
        int llc_size;
        int cache_ways_count;
        int cache_way_size;

        PQOSInterface *pqos;
        };

}  // namespace rdt

#endif //SNAP_PLUGIN_COLLECTOR_RDT_RDT_HPP
