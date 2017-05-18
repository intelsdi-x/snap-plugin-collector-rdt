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

#ifndef SNAP_PLUGIN_COLLECTOR_RDT_PQOS_HPP
#define SNAP_PLUGIN_COLLECTOR_RDT_PQOS_HPP

#include "pqos.h"

namespace rdt
{

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

}  // namespace rdt

#endif //SNAP_PLUGIN_COLLECTOR_RDT_PQOS_HPP
