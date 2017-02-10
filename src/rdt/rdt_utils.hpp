#ifndef SNAP_PLUGIN_COLLECTOR_RDT_RDT_UTILS_HPP
#define SNAP_PLUGIN_COLLECTOR_RDT_RDT_UTILS_HPP

#include "pqos.h"

namespace rdt {

    bool has_cmt_capability(const struct pqos_cap *capas);
    bool has_local_mbm_capability(const struct pqos_cap *capas);
    bool has_l3_cache_allocation_capabilities(const struct pqos_cap *capas);

}  // namespace rdt

#endif  // SNAP_PLUGIN_COLLECTOR_RDT_RDT_UTILS_HPP
