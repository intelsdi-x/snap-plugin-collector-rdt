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

#ifndef SNAP_PLUGIN_COLLECTOR_RDT_RDT_UTILS_HPP
#define SNAP_PLUGIN_COLLECTOR_RDT_RDT_UTILS_HPP

#include "pqos.h"

namespace rdt {
    bool has_cmt_capability(const struct pqos_cap *capas);
    bool has_local_mbm_capability(const struct pqos_cap *capas);
    bool has_remote_mbm_capability(const struct pqos_cap *capas);
    bool has_l3_cache_allocation_capabilities(const struct pqos_cap *capas);

}  // namespace rdt

#endif  // SNAP_PLUGIN_COLLECTOR_RDT_RDT_UTILS_HPP
