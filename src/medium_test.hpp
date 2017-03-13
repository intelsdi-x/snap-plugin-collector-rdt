#ifndef SNAP_PLUGIN_COLLECTOR_RDT_MEDIUM_TEST_HPP
#define SNAP_PLUGIN_COLLECTOR_RDT_MEDIUM_TEST_HPP

#include "pqos.h"
#include "gmock/gmock.h"
#include "rdt/rdt.hpp"

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

#endif // SNAP_PLUGIN_COLLECTOR_RDT_MEDIUM_TEST_HPP
