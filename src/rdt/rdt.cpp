// Copyright (c) 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdlib>
#include <sstream>

#include "rdt.hpp"
#include "rdt_utils.hpp"


namespace rdt
{
    Plugin::Namespace cmt_capability_ns = Plugin::Namespace({"intel", "rdt", "capabilities", "cmt_capability"});
    Plugin::Namespace mbm_local_monitoring_ns = Plugin::Namespace({"intel", "rdt", "capabilities", "mbm_local_monitoring"});
    Plugin::Namespace mbm_remote_monitoring_ns = Plugin::Namespace({"intel", "rdt", "capabilities", "mbm_remote_monitoring"});
    Plugin::Namespace l3ca_ns = Plugin::Namespace({"intel", "rdt", "capabilities", "cache_allocation"});
    Plugin::Namespace llc_size_ns = Plugin::Namespace({"intel", "rdt", "capabilities", "llc_size"});
    Plugin::Namespace cache_ways_count_ns = Plugin::Namespace({"intel", "rdt", "capabilities", "cache_ways_count"});
    Plugin::Namespace cache_way_size_ns = Plugin::Namespace({"intel", "rdt", "capabilities", "cache_way_size"});

Collector::Collector(PQOSInterface *pqos)
{
    struct pqos_config config = {0};
    const struct pqos_cpuinfo *p_cpu = NULL;
    const struct pqos_cap *p_cap = NULL;
    int ret = 0;
    this->pqos = pqos;

    memset(&config, 0, sizeof(config));
    config.fd_log = STDERR_FILENO;
    config.verbose = 0;

    ret = pqos->pqos_init(&config);
    if (ret != PQOS_RETVAL_OK)
    {
        std::stringstream ss;
        ss << "Error initializing PQoS library: " << ret << std::endl;
        fprintf(stderr, ss.str().c_str());
        throw Plugin::PluginException(ss.str());
    }

    /* Get CMT capability and CPU info pointer */
    ret = pqos->pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK)
    {
        pqos_fini();
        std::stringstream ss;
        ss << "Error initializing PQoS capabilities: " << ret << std::endl;
        fprintf(stderr, ss.str().c_str());
        throw Plugin::PluginException(ss.str());
    }

        this->cmt_capability = has_cmt_capability(p_cap);
        this->mbm_local_capability = has_local_mbm_capability(p_cap);
        this->mbm_remote_capability = has_remote_mbm_capability(p_cap);
        this->l3ca_capability = has_l3_cache_allocation_capabilities(p_cap);

    this->core_count = p_cpu->num_cores;
    this->cache_way_size = p_cpu->l3.way_size;
    this->cache_ways_count = p_cpu->l3.num_ways;
    this->llc_size = p_cpu->l3.total_size;

    this->is_monitoring_active = false;
}

Collector::~Collector()
{
    if (this->is_monitoring_active)
    {
        while (!this->groups.empty())
        {
            pqos->pqos_mon_stop(this->groups.back());
            free(this->groups.back());
            this->groups.pop_back();
        }
    }
    pqos->pqos_fini();
}

const Plugin::ConfigPolicy Collector::get_config_policy()
{
    return Plugin::ConfigPolicy{};
}

std::vector<Plugin::Metric> Collector::get_metric_types(Plugin::Config cfg)
{
    std::vector<Plugin::Metric> metrics;

    // CMT Count.
    if (this->cmt_capability)
    {
        for (int cpu_index = 0; cpu_index < this->core_count; cpu_index++)
        {
            std::string core_id = std::to_string(cpu_index);
            metrics.push_back(Plugin::Metric(
                Plugin::Namespace({"intel","rdt","llc_occupancy"}).
                    add_dynamic_element("core_id","Cache occupancy for core_id").
                    add_static_element("bytes"),
                "bytes",
                "Total LLC Occupancy of CPU " + core_id + " in bytes."));
            metrics.push_back(Plugin::Metric(
                Plugin::Namespace({"intel","rdt","llc_occupancy"}).
                    add_dynamic_element("core_id","Cache occupancy for core_id").
                    add_static_element("percentage"),
                "percentage",
                "Total LLC Occupancy of CPU " + core_id + " in bytes."));
        }
    }

    // MBM metrics
    if (this->mbm_local_capability || this->mbm_remote_capability) {
        for (int cpu_index = 0; cpu_index < this->core_count; cpu_index++) {
            std::string core_id = std::to_string(cpu_index);

            if (this->mbm_local_capability) {
                metrics.push_back(Plugin::Metric(
                    Plugin::Namespace({"intel","rdt","memory_bandwidth","local"}).
                        add_dynamic_element("core_id","core_id to gather local memory bandwidth usage for").
                        add_static_element("bytes"),
                    "bytes",
                    "Local memory bandwidth usage for CPU " + core_id + " in bytes."));
            }
            if (this->mbm_remote_capability) {
                metrics.push_back(Plugin::Metric(
                    Plugin::Namespace({"intel","rdt","memory_bandwidth","remote"}).
                        add_dynamic_element("core_id","core_id to gather remote memory bandwidth usage for").
                        add_static_element("bytes"),
                    "bytes",
                    "Remote memory bandwidth usage for CPU " + core_id + " in bytes."));
                
                metrics.push_back(Plugin::Metric(
                    Plugin::Namespace({"intel","rdt","memory_bandwidth","total"}).
                        add_dynamic_element("core_id","core_id to gather total memory bandwidth usage for").
                        add_static_element("bytes"),
                    "bytes",
                    "Total memory bandwidth usage for CPU " + core_id + " in bytes."));
            }
        }
    }

    // Monitoring capabilities.
    metrics.push_back(Plugin::Metric(
            cmt_capability_ns,
            "bool",
            "This CPU supports LLC Cache Monitoring."
    ));

    metrics.push_back(Plugin::Metric(
            mbm_local_monitoring_ns,
            "bool",
            "This CPU supports Local Memory Bandwidth Monitoring."
    ));

    metrics.push_back(Plugin::Metric(
            mbm_remote_monitoring_ns,
            "bool",
            "This CPU supports Remote Memory Bandwidth Monitoring."
    ));

    metrics.push_back(Plugin::Metric(
            rdt::l3ca_ns,
            "bool",
            "This CPU supports L3CA capabilities."
    ));


    // CAT Capabilities.
    metrics.push_back(Plugin::Metric(
        l3ca_ns,
        "bytes",
        "LLC Size."));

    metrics.push_back(Plugin::Metric(
        cache_ways_count_ns,
        "bytes",
        "Number of cache ways in Last Level Cache."));

    metrics.push_back(Plugin::Metric(
        cache_way_size_ns,
        "bytes",
        "Size of cache way in Last Level Cache."));

    return metrics;
}

std::vector<Plugin::Metric> Collector::collect_metrics(std::vector<Plugin::Metric> &mts)
{
        std::vector<Plugin::Metric> metrics;
        // Load metrics.
        if (!this->is_monitoring_active) {
            this->setup_monitoring();
            usleep(100);
        }
        this->poll_metrics();

        if (this->cmt_capability) {
            std::vector<Plugin::Metric> monitoring_metrics = this->get_cmt_metrics();
            std::move(monitoring_metrics.begin(), monitoring_metrics.end(), std::back_inserter(metrics));
        }

        if (this->mbm_local_capability || this->mbm_remote_capability) {
            std::vector<Plugin::Metric> monitoring_metrics = this->get_mbm_metrics();
            std::move(monitoring_metrics.begin(), monitoring_metrics.end(), std::back_inserter(metrics));
        }

        std::vector<Plugin::Metric> capabilities = get_capabilities_metrics();
        std::move(capabilities.begin(), capabilities.end(), std::back_inserter(metrics));

    const auto now = std::chrono::system_clock::now();
    for (auto &metric : metrics)
    {
        metric.set_last_advertised_time(now);
        metric.set_timestamp(now);
    }

    return metrics;
}

std::vector<Plugin::Metric> Collector::get_capabilities_metrics()
{
    std::vector<Plugin::Metric> capabilities;

    Plugin::Metric cmt_capa_metric;
    cmt_capa_metric.set_ns(cmt_capability_ns);
    cmt_capa_metric.set_data(this->cmt_capability);
    capabilities.push_back(cmt_capa_metric);

    Plugin::Metric mbm_local_capa_metric;
    mbm_local_capa_metric.set_ns(mbm_local_monitoring_ns);
    mbm_local_capa_metric.set_data(this->mbm_local_capability);
    capabilities.push_back(mbm_local_capa_metric);

    Plugin::Metric mbm_remote_capa_metric;
    mbm_remote_capa_metric.set_ns(mbm_remote_monitoring_ns);
    mbm_remote_capa_metric.set_data(this->mbm_remote_capability);
    capabilities.push_back(mbm_remote_capa_metric);

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

void Collector::setup_monitoring()
{
    fprintf(stderr, "Resetting all RMIDs\n");
    int result = pqos->pqos_mon_reset();
    if (result != PQOS_RETVAL_OK)
    {
        std::stringstream ss;
        ss << "Could not reset PQoS RMIDs: " << result << std::endl;
        fprintf(stderr, ss.str().c_str());
        throw Plugin::PluginException(ss.str());
    }
    this->groups.clear();
    this->groups.reserve(static_cast<unsigned long>(this->core_count));

    for (int group_id = 0; group_id < this->core_count; group_id++)
    {
        pqos_mon_data *mon_data_group = (pqos_mon_data *)calloc(1, sizeof(pqos_mon_data));
        if (mon_data_group == nullptr)
        {
            throw Plugin::PluginException("Could not allocate memory for monitoring data structure");
        }
        this->groups.push_back(mon_data_group);
    }

        int eventsMask = PQOS_MON_EVENT_L3_OCCUP;
        if (this->mbm_remote_capability) {
            eventsMask = eventsMask | PQOS_MON_EVENT_RMEM_BW | PQOS_MON_EVENT_TMEM_BW;
        }

        if (this->mbm_local_capability) {
            eventsMask = eventsMask | PQOS_MON_EVENT_LMEM_BW;
        }
        enum pqos_mon_event events = (enum pqos_mon_event)eventsMask;

    for (unsigned int group_id = 0; group_id < this->core_count; group_id++)
    {
        const unsigned int monitored_cores_per_group = 1;
        unsigned int monitored_cores_ids[monitored_cores_per_group] = {group_id};
        void *context = NULL;
        int result = pqos->pqos_mon_start(
            monitored_cores_per_group,
            monitored_cores_ids,
            events,
            context,
            this->groups[group_id]);
        if (result != PQOS_RETVAL_OK)
        {
            std::stringstream ss;
            ss << "Could not start PQoS monitoring: " << result << std::endl;
            fprintf(stderr, ss.str().c_str());
            throw Plugin::PluginException(ss.str());
        }
    }

    this->is_monitoring_active = true;
    return;
}

void Collector::poll_metrics()
{
    int result = pqos->pqos_mon_poll(
       &this->groups[0],
       static_cast<unsigned int>(this->groups.size()));
    if (result != PQOS_RETVAL_OK)
    {
        std::stringstream ss;
        ss << "Could not poll PQoS metrics: " << result << std::endl;
        fprintf(stderr, ss.str().c_str());
        throw Plugin::PluginException(ss.str());
    }
    return;
}

std::vector<Plugin::Metric> Collector::get_cmt_metrics() {
        std::vector<Plugin::Metric> metrics;

    for (auto &group : this->groups)
    {
        Plugin::Metric cmt_bytes(
            Plugin::Namespace({"intel","rdt","llc_occupancy"}).
                add_static_element(std::to_string(group->cores[0])).
                add_static_element("bytes"),
            "","");

        double cmt_data = static_cast<double>(group->values.llc);
        cmt_bytes.set_data(cmt_data);

        Plugin::Metric cmt_percentage(
            Plugin::Namespace({"intel","rdt","llc_occupancy"}).
                add_static_element(std::to_string(group->cores[0])).
                add_static_element("percentage"),
            "","");
        cmt_percentage.set_data((static_cast<double>(cmt_data) / static_cast<double>(this->llc_size)) * 100);

        metrics.push_back(cmt_bytes);
        metrics.push_back(cmt_percentage);
        }
        return metrics;
    }

    std::vector<Plugin::Metric> Collector::get_mbm_metrics() {
        std::vector<Plugin::Metric> metrics;

        for(auto& group : this->groups) {
            Plugin::Metric mbw;
            std::string coreId = std::to_string(group->cores[0]);

            if (this->mbm_local_capability) {
                Plugin::Namespace local_ns({"intel","rdt","memory_bandwidth","local",coreId,"bytes"});
                mbw.set_ns(local_ns);
                double local_mbw = static_cast<double>(group->values.mbm_local_delta);
                mbw.set_data(local_mbw);
                metrics.push_back(mbw);
            }

            if (this->mbm_remote_capability) {
                Plugin::Namespace remote_ns({"intel","rdt","memory_bandwidth","remote",coreId,"bytes"});
                mbw.set_ns(remote_ns);
                double remote_mbw = static_cast<double>(group->values.mbm_remote_delta);
                mbw.set_data(remote_mbw);
                metrics.push_back(mbw);

                Plugin::Namespace total_ns({"intel","rdt","memory_bandwidth","total",coreId,"bytes"});
                mbw.set_ns(total_ns);
                double total_mbw = static_cast<double>(group->values.mbm_total_delta);
                mbw.set_data(total_mbw);
                metrics.push_back(mbw);
            }
        }
        return metrics;
    }

Plugin::Meta Collector::get_plugin_meta()
{
    return Plugin::Meta(Plugin::Collector, this->name, this->version);
}

} // namespace rdt
