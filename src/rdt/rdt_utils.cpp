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

#include <vector>
#include <string>
#include <sstream>

#include "pqos.h"
#include "snap/metric.h"
#include "rdt_utils.hpp"


namespace rdt {

    bool has_monitoring_capability(const struct pqos_cap *capas, const pqos_mon_event event) {
        for (int capa_index = 0; capa_index < capas->num_cap; capa_index++) {
            if (capas->capabilities[capa_index].type == PQOS_CAP_TYPE_MON) {
                int supported_events = capas->capabilities[capa_index].u.mon->num_events;
                for (int event_idx = 0; event_idx < supported_events; event_idx++) {
                    if (event == capas->capabilities[capa_index].u.mon->events[event_idx].type) {
                       return true;
                    }
                }
            }
        }
        return false;
    }

    bool has_cmt_capability(const struct pqos_cap *capas) {
        return has_monitoring_capability(capas, PQOS_MON_EVENT_L3_OCCUP);
    }

    bool has_local_mbm_capability(const struct pqos_cap *capas) {
        return has_monitoring_capability(capas, PQOS_MON_EVENT_LMEM_BW);
    }

    bool has_remote_mbm_capability(const struct pqos_cap *capas) {
        return has_monitoring_capability(capas, PQOS_MON_EVENT_RMEM_BW);
    }

    bool has_l3_cache_allocation_capabilities(const struct pqos_cap *capas) {
        for (int capability_index = 0; capability_index < capas->num_cap; capability_index++) {
            if (capas->capabilities[capability_index].type == PQOS_CAP_TYPE_L3CA) {
                return true;
            }
        }
        return false;
    }

    std::string merge_namespace(const std::vector<Plugin::Metric::NamespaceElement>& ns)
    {
        std::ostringstream oss;
        for (auto& element : ns)
        {
            oss << element.value << "/";
        }
        return oss.str();
    }

    bool isMetricNamespaceEqual(
            const std::vector<Plugin::Metric::NamespaceElement>& left,
            const std::vector<Plugin::Metric::NamespaceElement>& right)
    {
        std::string left_namespace = merge_namespace(left);
        std::string right_namespace = merge_namespace(right);

        bool is_string_equal = (0 == left_namespace.compare(right_namespace));
        return is_string_equal;
    }

    Plugin::Metric* find_metric_with_namespace(Plugin::Metric requested_metric, std::vector<Plugin::Metric> metrics)
    {
        std::string requested_namespace = merge_namespace(requested_metric.ns());
        for (auto& metric : metrics)
        {
            std::string metric_ns = merge_namespace(metric.ns());
            if (requested_namespace.compare(metric_ns) == 0)
            {
                return &metric;
            }
        }
        return nullptr;
    }


}  // namespace rdt
