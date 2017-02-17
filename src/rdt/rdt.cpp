#include <cstdlib>
#include <sstream>

#include "rdt.hpp"
#include "rdt_utils.hpp"

namespace rdt {

    Collector::Collector() {
        struct pqos_config config = {0};
        const struct pqos_cpuinfo *p_cpu = NULL;
        const struct pqos_cap *p_cap = NULL;
        int ret = 0;

        memset(&config, 0, sizeof(config));
        config.fd_log = STDERR_FILENO;
        config.verbose = 0;

        ret = pqos_init(&config);
        if (ret != PQOS_RETVAL_OK) {
            std::stringstream ss;
            ss << "Error initializing PQoS library: " << ret << std::endl;
            fprintf(stderr, ss.str().c_str());
            throw Plugin::PluginException(ss.str());
        }

        /* Get CMT capability and CPU info pointer */
        ret = pqos_cap_get(&p_cap, &p_cpu);
        if (ret != PQOS_RETVAL_OK) {
            pqos_fini();
            std::stringstream ss;
            ss << "Error initializing PQoS capabilities: " << ret << std::endl;
            fprintf(stderr, ss.str().c_str());
            throw Plugin::PluginException(ss.str());
        }

        this->cmt_capability = has_cmt_capability(p_cap);
        this->mbm_local_capability = has_local_mbm_capability(p_cap);
        this->l3ca_capability = has_l3_cache_allocation_capabilities(p_cap);

        this->core_count = p_cpu->num_cores;
        this->cache_way_size = p_cpu->l3.way_size;
        this->cache_ways_count = p_cpu->l3.num_ways;
        this->llc_size = p_cpu->l3.total_size;

        this->is_monitoring_active = false;
    }

    Collector::~Collector() {
        if (this->is_monitoring_active) {
            while (!this->groups.empty()){
                pqos_mon_stop(this->groups.back());
                free(this->groups.back());
                this->groups.pop_back();
            }
        }
        pqos_fini();
    }

    const Plugin::ConfigPolicy Collector::get_config_policy() {
        return Plugin::ConfigPolicy{};
    }

    std::vector<Plugin::Metric> Collector::get_metric_types(Plugin::Config cfg) {
        std::vector<Plugin::Metric> namespaces;

        // CMT Count.
        if (this->cmt_capability) {
            for (int cpu_index = 0; cpu_index < this->core_count; cpu_index++) {
                Plugin::Metric::NamespaceElement dynamicCoreIdElement;
                dynamicCoreIdElement.value = "*";
                dynamicCoreIdElement.name = "core_id";
                dynamicCoreIdElement.description = "Cache occupancy for core_id";

                std::string core_id = std::to_string(cpu_index);
                Plugin::Metric llc_occupancy_bytes(
                        {{"intel"}, {"rdt"}, {"llc_occupancy"}, dynamicCoreIdElement, {"bytes"}},
                        "bytes",
                        "Total LLC Occupancy of CPU " + core_id + " in bytes."
                );
                Plugin::Metric llc_occupancy_percentage(
                        {{"intel"}, {"rdt"}, {"llc_occupancy"}, dynamicCoreIdElement, {"percentage"}},
                        "percentage",
                        "Total LLC Occupancy of CPU " + core_id + " in bytes."
                );

                namespaces.push_back(llc_occupancy_bytes);
                namespaces.push_back(llc_occupancy_percentage);
            }
        }

        // Monitoring capabilities.
        namespaces.push_back(Plugin::Metric(
                cmt_capability_ns,
                "bool",
                "This CPU supports LLC Cache Monitoring."
        ));

        namespaces.push_back(Plugin::Metric(
                mbm_local_monitoring_ns,
                "bool",
                "This CPU supports Local Memory Bandwidth Monitoring."
        ));


        // CAT Capabilities.
        namespaces.push_back(Plugin::Metric(
                llc_size_ns,
                "bytes",
                "LLC Size."
        ));

        namespaces.push_back(Plugin::Metric(
                cache_ways_count_ns,
                "bytes",
                "Number of cache ways in Last Level Cache."
        ));

        namespaces.push_back(Plugin::Metric(
                cache_way_size_ns,
                "bytes",
                "Size of cache way in Last Level Cache."
        ));

        return namespaces;
    }

    void Collector::collect_metrics(std::vector<Plugin::Metric> &metrics) {
        metrics.clear();

        if (this->cmt_capability) {
            // Load CMT metrcs.
            if (!this->is_monitoring_active) {
                this->setup_cmt_monitoring();
                usleep(100);
            }
            std::vector<Plugin::Metric> monitoring_metrics = this->get_cmt_metrics();
            std::move(monitoring_metrics.begin(), monitoring_metrics.end(), std::back_inserter(metrics));
        }

        std::vector<Plugin::Metric> capabilities = get_capabilities_metrics();
        std::move(capabilities.begin(), capabilities.end(), std::back_inserter(metrics));

        const auto now = std::chrono::system_clock::now();
        for (auto& metric : metrics) {
            metric.set_last_advertised_time(now);
            metric.set_timestamp(now);
        }

        return;
    }

    std::vector<Plugin::Metric> Collector::get_capabilities_metrics() {
        std::vector<Plugin::Metric> capabilities;

        Plugin::Metric cmt_capa_metric;
        cmt_capa_metric.set_ns(cmt_capability_ns);
        cmt_capa_metric.set_data(this->cmt_capability);
        capabilities.push_back(cmt_capa_metric);

        Plugin::Metric mbm_local_capa_metric;
        mbm_local_capa_metric.set_ns(mbm_local_monitoring_ns);
        mbm_local_capa_metric.set_data(this->mbm_local_capability);
        capabilities.push_back(mbm_local_capa_metric);

        Plugin::Metric l3ca_capa_metric;
        l3ca_capa_metric.set_ns(l3ca_ns);
        l3ca_capa_metric.set_data(this->l3ca_capability);
        capabilities.push_back(l3ca_capa_metric);

        Plugin::Metric llc_size_metric;
        llc_size_metric.set_ns(llc_size_ns);
        llc_size_metric.set_data(this->llc_size);
        capabilities.push_back(llc_size_metric);

        Plugin::Metric cache_ways_count_metric;
        cache_ways_count_metric.set_ns(cache_ways_count_ns);
        cache_ways_count_metric.set_data(this->cache_ways_count);
        capabilities.push_back(cache_ways_count_metric);

        Plugin::Metric cache_way_size_metric;
        cache_way_size_metric.set_ns(cache_way_size_ns);
        cache_way_size_metric.set_data(this->cache_way_size);
        capabilities.push_back(cache_way_size_metric);

        return capabilities;
    }

    void Collector::setup_cmt_monitoring() {
        fprintf(stderr, "Resetting all RMIDs\n");
        int result = pqos_mon_reset();
        if (result != PQOS_RETVAL_OK) {
            std::stringstream ss;
            ss << "Could not reset PQoS RMIDs: " << result << std::endl;
            fprintf(stderr, ss.str().c_str());
            throw Plugin::PluginException(ss.str());
        }
        this->groups.clear();
        this->groups.reserve(static_cast<unsigned long>(this->core_count));

        for (int group_id = 0; group_id < this->core_count; group_id++) {
            pqos_mon_data* mon_data_group = (pqos_mon_data*) calloc(1, sizeof(pqos_mon_data));
            if (mon_data_group == nullptr) {
                throw Plugin::PluginException("Could not allocate memory for monitoring data structure");
            }
            this->groups.push_back(mon_data_group);
        }

        enum pqos_mon_event events = PQOS_MON_EVENT_L3_OCCUP;

        for (unsigned int group_id = 0; group_id < this->core_count; group_id++) {
            const unsigned int monitored_cores_per_group = 1;
            unsigned int monitored_cores_ids[monitored_cores_per_group] = {group_id};
            void *context = NULL;
            int result = pqos_mon_start(
                    monitored_cores_per_group,
                    monitored_cores_ids,
                    events,
                    context,
                    this->groups[group_id]);
            if (result != PQOS_RETVAL_OK) {
                std::stringstream ss;
                ss << "Could not start PQoS monitoring: " << result << std::endl;
                fprintf(stderr, ss.str().c_str());
                throw Plugin::PluginException(ss.str());
            }
        }

        this->is_monitoring_active = true;

        return;
    }

    void Collector::poll_metrics() {
        int result = pqos_mon_poll(
                &this->groups[0],
                static_cast<unsigned int>(this->groups.size()));
        if (result != PQOS_RETVAL_OK) {
            std::stringstream ss;
            ss << "Could not poll PQoS metrics: " << result << std::endl;
            fprintf(stderr, ss.str().c_str());
            throw Plugin::PluginException(ss.str());
        }
        return;
    }

    std::vector<Plugin::Metric> Collector::get_cmt_metrics() {
        std::vector<Plugin::Metric> metrics;
        this->poll_metrics();

        for (auto& group : this->groups){
            Plugin::Metric cmt_bytes;

            Plugin::Metric::NamespaceElement dynamicCoreIdElement;
            dynamicCoreIdElement.value = std::to_string(group->cores[0]);
            dynamicCoreIdElement.name = "core_id";
            dynamicCoreIdElement.description = "Cache occupancy for core_id";

            int cmt_data = static_cast<int>(group->values.llc);
            cmt_bytes.set_data(cmt_data);
            cmt_bytes.set_ns({{"intel"}, {"rdt"}, {"llc_occupancy"}, dynamicCoreIdElement, {"bytes"}});

            Plugin::Metric cmt_percentage;
            cmt_percentage.set_data((static_cast<double>(cmt_data) / static_cast<double>(this->llc_size)) * 100);
            cmt_percentage.set_ns({{"intel"}, {"rdt"}, {"llc_occupancy"}, dynamicCoreIdElement, {"percentage"}});

            metrics.push_back(cmt_bytes);
            metrics.push_back(cmt_percentage);
        }
        return metrics;
    }

    Plugin::Meta Collector::get_plugin_meta() {
        return Plugin::Meta(Plugin::Collector, this->name, this->version);
    }

}  // namespace rdt
