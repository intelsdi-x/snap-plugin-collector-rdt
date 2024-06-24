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

#ifndef SNAP_PLUGIN_COLLECTOR_RDT_MEDIUM_TEST_HPP
#define SNAP_PLUGIN_COLLECTOR_RDT_MEDIUM_TEST_HPP

#include "pqos.h"
#include "gmock/gmock.h"
#include "rdt/rdt.hpp"
#include "rdt/pqos.hpp"

namespace rdt
{

typedef struct {
    double expected_value;
    bool is_float;
    bool is_bool;
} rdt_metric_data;
typedef std::unordered_map<std::string, rdt_metric_data> rdt_metric_map;

class PQOSMock : public PQOSInterface
{
public:
    MOCK_METHOD1(pqos_init, int(const pqos_config *config));
    MOCK_METHOD0(pqos_init, int());
    MOCK_METHOD2(pqos_cap_get, int(const struct pqos_cap **cap, const struct pqos_cpuinfo **cpu));
    MOCK_METHOD0(pqos_fini, int());
    MOCK_METHOD5(pqos_mon_start, int(const unsigned num_cores, const unsigned *cores, const enum pqos_mon_event event, void *context, struct pqos_mon_data *group));
    MOCK_METHOD0(pqos_mon_reset, int());
    MOCK_METHOD1(pqos_mon_stop, int(struct pqos_mon_data *group));
    MOCK_METHOD2(pqos_mon_poll, int(struct pqos_mon_data **groups, const unsigned num_groups));
};

}  // namespace rdt

#endif // SNAP_PLUGIN_COLLECTOR_RDT_MEDIUM_TEST_HPP
