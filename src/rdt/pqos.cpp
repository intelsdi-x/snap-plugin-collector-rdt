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

#include "pqos.hpp"

namespace rdt
{

int PQOS::pqos_init(const pqos_config *config)
{
    return ::pqos_init(config);
}

int PQOS::pqos_cap_get(const struct pqos_cap **cap, const struct pqos_cpuinfo **cpu)
{
    return ::pqos_cap_get(cap, cpu);
}

int PQOS::pqos_fini()
{
    return ::pqos_fini();
}

int PQOS::pqos_mon_start(const unsigned num_cores, const unsigned *cores, const enum pqos_mon_event event, void *context, struct pqos_mon_data *group)
{
    return ::pqos_mon_start(num_cores, cores, event, context, group);
}

int PQOS::pqos_mon_reset()
{
    return ::pqos_mon_reset();
}

int PQOS::pqos_mon_stop(struct pqos_mon_data *group)
{
    return ::pqos_mon_stop(group);
}

int PQOS::pqos_mon_poll(struct pqos_mon_data **groups, const unsigned num_groups)
{
    return ::pqos_mon_poll(groups, num_groups);
}

}  // namespace rdt
