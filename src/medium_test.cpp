#include "medium_test.hpp"
#include "rdt/rdt_utils.hpp"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgumentPointee;
using ::testing::SaveArgPointee;
using ::testing::Invoke;

bool is_prefix(std::string const& a, std::string const& b) {
    auto res = std::mismatch(a.begin(), a.end(), b.begin());
    return res.first == a.end();
}

// Mock capabilities object
pqos_cap *mock_caps(std::vector<pqos_mon_event> events)
{
    // Mock capability list
    auto pqosCapMock = new pqos_capability;

    pqosCapMock->type = PQOS_CAP_TYPE_MON;
    pqosCapMock->u.mon = (pqos_cap_mon*) malloc(sizeof(pqos_cap_mon) + sizeof(pqos_monitor) * events.size());
    pqosCapMock->u.mon->num_events = events.size();
    for (int i = 0; i < pqosCapMock->u.mon->num_events; i++)
        pqosCapMock->u.mon->events[i] = {events[i]};

    // Assign capability list to capabilities object
    auto result = (pqos_cap*) malloc(sizeof(pqos_cap) + sizeof(pqos_capability));
    result->num_cap = 1;
    result->capabilities[0] = *pqosCapMock;

    return result;
}

// Mock CPU object
pqos_cpuinfo *mock_cpu(unsigned num_cores) {
    auto mockCpuInfo = new pqos_cpuinfo;
    mockCpuInfo->num_cores = num_cores;
    mockCpuInfo->l3.way_size = 10 * num_cores;
    mockCpuInfo->l3.num_ways = 200 * num_cores;
    mockCpuInfo->l3.total_size = 300 * num_cores;
    return mockCpuInfo;
}

// Mock monitor group data
void mock_mon_poll(struct pqos_mon_data **groups, const unsigned num_groups) {
    for (unsigned i = 0; i < num_groups; i++) {
        groups[i]->cores = new unsigned(i);
        groups[i]->poll_ctx = new pqos_mon_poll_ctx;
        groups[i]->values.llc = 100 * (i+1);
    }
}

// Mock metrics to collect_metrics
std::vector<Plugin::Metric> mock_metrics(unsigned cpu_cores) {
    auto metrics = std::vector<Plugin::Metric>{
        Plugin::Metric(rdt::cmt_capability_ns, "bool", "This CPU supports LLC Cache Monitoring."),
        Plugin::Metric(rdt::mbm_local_monitoring_ns, "bool", "This CPU supports Local Memory Bandwidth Monitoring."),
        Plugin::Metric(rdt::llc_size_ns, "bytes", "LLC Size."),
        Plugin::Metric(rdt::cache_ways_count_ns, "bytes", "Number of cache ways in Last Level Cache."),
        Plugin::Metric(rdt::cache_way_size_ns, "bytes", "Size of cache way in Last Level Cache.")
    };

    for (unsigned i = 0; i < cpu_cores; i++)
    {
        Plugin::Metric::NamespaceElement dynamicCoreIdElement;
        dynamicCoreIdElement.value = "*";
        dynamicCoreIdElement.name = "core_id";
        dynamicCoreIdElement.description = "Cache occupancy for core_id";

        std::string core_id = std::to_string(i);
        Plugin::Metric llc_occupancy_bytes(
            {{"intel"}, {"rdt"}, {"llc_occupancy"}, dynamicCoreIdElement, {"bytes"}},
            "bytes",
            "Total LLC Occupancy of CPU " + core_id + " in bytes.");
        Plugin::Metric llc_occupancy_percentage(
            {{"intel"}, {"rdt"}, {"llc_occupancy"}, dynamicCoreIdElement, {"percentage"}},
            "percentage",
            "Total LLC Occupancy of CPU " + core_id + " in bytes.");

        metrics.push_back(llc_occupancy_bytes);
        metrics.push_back(llc_occupancy_percentage);
    }

    return metrics;
}

// Test PQOS init failure on collector construct
TEST(CollectorContructor, TestPQOSInitFailure) {
    PQOSMock p_mock;

    // Checking PQOS init failure
    EXPECT_CALL(p_mock, pqos_init(_)).WillOnce(Return(PQOS_RETVAL_ERROR));
    EXPECT_CALL(p_mock, pqos_cap_get(_, _)).Times(0);
    EXPECT_CALL(p_mock, pqos_fini()).Times(0);

    std::string msg = "";
    try {
        rdt::Collector *rdt = new rdt::Collector(&p_mock);
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
    } catch(Plugin::PluginException &e) {
        msg = e.what();
    }

    EXPECT_TRUE(is_prefix("Error initializing PQoS library", msg));
}

// Test get PQOS capabilities failure on collector construct
TEST(CollectorContructor, TestPQOSCapGetFailure) {
    PQOSMock p_mock;

    // Checking PQOS init failure
    EXPECT_CALL(p_mock, pqos_init(_)).WillOnce(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillOnce(Return(PQOS_RETVAL_ERROR));
    EXPECT_CALL(p_mock, pqos_fini()).Times(0);

    std::string msg = "";
    try {
        rdt::Collector *rdt = new rdt::Collector(&p_mock);
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
    } catch(Plugin::PluginException &e) {
        msg = e.what();
    }

    EXPECT_TRUE(is_prefix("Error initializing PQoS capabilities", msg));
}

// Test successfull collector construct
TEST(CollectorContructor, TestConstructorSuccess) {
    PQOSMock p_mock;
    pqos_cap *p_cap = mock_caps(std::vector<pqos_mon_event>{});
    pqos_cpuinfo *p_cpu = mock_cpu(1);

    // Checking PQOS init failure
    EXPECT_CALL(p_mock, pqos_init(_)).WillOnce(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillOnce(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));
    EXPECT_CALL(p_mock, pqos_fini()).Times(0);

    bool error = false;
    try {
        rdt::Collector *rdt = new rdt::Collector(&p_mock);
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
    } catch(Plugin::PluginException &e) {
        error = true;
    }

    EXPECT_FALSE(error);
    delete(p_cpu);
    delete(p_cap);
}

// Test get metric types with CMT capability available
TEST(GetMetricTypesTest, TestMetricCountMultiCpu) {
    PQOSMock p_mock;
    pqos_cap *p_cap;
    pqos_cpuinfo *p_cpu;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_L3_OCCUP, PQOS_MON_EVENT_LMEM_BW, PQOS_MON_EVENT_TMEM_BW});
    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    for (int num_cores = 1; num_cores <= 4; num_cores++) {
        p_cpu = mock_cpu(num_cores);

        EXPECT_TRUE(rdt::has_cmt_capability(p_cap));
        EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillOnce(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));
        rdt::Collector *rdt = new rdt::Collector(&p_mock);
        rpc::ConfigMap map;
        Plugin::Config config(map);
        auto metric_types = rdt->get_metric_types(config);

        EXPECT_EQ(metric_types.size(), 5 + num_cores * 2);
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
        delete(p_cpu);
    }

    delete(p_cap);
}

// Test get metric types without CMT capability available
TEST(GetMetricTypesTest, TestMetricCountMultiCpuNoCmtCap) {
    PQOSMock p_mock;
    pqos_cap *p_cap;
    pqos_cpuinfo *p_cpu;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_LMEM_BW, PQOS_MON_EVENT_TMEM_BW});
    EXPECT_FALSE(rdt::has_cmt_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    for (int num_cores = 1; num_cores <= 4; num_cores++) {
        p_cpu = mock_cpu(num_cores);

        EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillOnce(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));
        rdt::Collector *rdt = new rdt::Collector(&p_mock);
        rpc::ConfigMap map;
        Plugin::Config config(map);
        auto metric_types = rdt->get_metric_types(config);

        EXPECT_EQ(metric_types.size(), 5);
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
        delete(p_cpu);
    }

    delete(p_cap);
}

// Test collecting metrics without CMT capability
TEST(CollectMetricsTest, TestCollectNoCmtCap) {
    PQOSMock p_mock;
    pqos_cap *p_cap;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_LMEM_BW, PQOS_MON_EVENT_TMEM_BW});
    EXPECT_FALSE(rdt::has_cmt_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    for (int num_cores = 1; num_cores <= 4; num_cores++)
    {
        pqos_cpuinfo *p_cpu = mock_cpu(num_cores);
        EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillRepeatedly(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));

        rdt::Collector *rdt = new rdt::Collector(&p_mock);

        bool error = false;
        auto metrics = mock_metrics(num_cores);
        try
        {
            rdt->collect_metrics(metrics);
        } catch(Plugin::PluginException &e) {
            error = true;
        }

        EXPECT_FALSE(error);
        EXPECT_EQ(metrics.size(), 6);
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
        delete(p_cpu);
    }

    delete(p_cap);
}

// Test collecting metrics with pqos_mon_start failure
TEST(CollectMetricsTest, TestCollectMonStartErr) {
    PQOSMock p_mock;
    pqos_cap *p_cap;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_L3_OCCUP, PQOS_MON_EVENT_LMEM_BW, PQOS_MON_EVENT_TMEM_BW});
    EXPECT_TRUE(rdt::has_cmt_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_reset()).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_poll(_, _)).WillRepeatedly(DoAll(Invoke(mock_mon_poll), Return(PQOS_RETVAL_OK)));

    for (int num_cores = 1; num_cores <= 4; num_cores++)
    {
        pqos_cpuinfo *p_cpu = mock_cpu(num_cores);

        EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillRepeatedly(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));
        EXPECT_CALL(p_mock, pqos_mon_start(_, _, _, _, _)).WillOnce(Return(PQOS_RETVAL_ERROR));
        rdt::Collector *rdt = new rdt::Collector(&p_mock);

        std::string msg;
        auto metrics = mock_metrics(num_cores);
        try
        {
            rdt->collect_metrics(metrics);
        } catch(Plugin::PluginException &e) {
            msg = e.what();
        }
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
        delete(p_cpu);

        EXPECT_TRUE(is_prefix("Could not start PQoS monitoring", msg));
    }

    delete(p_cap);
}

// Test collecting metrics with pqos_mon_reset failure
TEST(CollectMetricsTest, TestCollectMonResetErr) {
    PQOSMock p_mock;
    pqos_cap *p_cap;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_L3_OCCUP, PQOS_MON_EVENT_LMEM_BW, PQOS_MON_EVENT_TMEM_BW});
    EXPECT_TRUE(rdt::has_cmt_capability(p_cap));
    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    for (int num_cores = 1; num_cores <= 4; num_cores++)
    {
        pqos_cpuinfo *p_cpu = mock_cpu(num_cores);

        EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillRepeatedly(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));
        EXPECT_CALL(p_mock, pqos_mon_reset()).WillOnce(Return(PQOS_RETVAL_ERROR));

        rdt::Collector *rdt = new rdt::Collector(&p_mock);

        std::string msg;
        auto metrics = mock_metrics(num_cores);
        try {
            rdt->collect_metrics(metrics);
        } catch(Plugin::PluginException &e) {
            msg = e.what();
        }

        EXPECT_TRUE(is_prefix("Could not reset PQoS RMIDs", msg));
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
        delete(p_cpu);
    }

    delete(p_cap);
}

// Test collecting metrics successfull
TEST(CollectMetricsTest, TestCollectSuccess) {
    PQOSMock p_mock;
    pqos_cap *p_cap;
    pqos_cpuinfo *p_cpu;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_L3_OCCUP, PQOS_MON_EVENT_LMEM_BW, PQOS_MON_EVENT_TMEM_BW});
    EXPECT_TRUE(rdt::has_cmt_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_reset()).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_poll(_, _)).WillRepeatedly(DoAll(Invoke(mock_mon_poll), Return(PQOS_RETVAL_OK)));
    EXPECT_CALL(p_mock, pqos_mon_start(_, _, _, _, _)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    for (int num_cores = 1; num_cores <= 4; num_cores++) {
        p_cpu = mock_cpu(num_cores);
        EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillRepeatedly(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));

        rdt::Collector *rdt = new rdt::Collector(&p_mock);
        bool error = false;
        auto metrics = mock_metrics(num_cores);
        try {
            rdt->collect_metrics(metrics);
        } catch(Plugin::PluginException &e) {
            error = true;
        }

        EXPECT_FALSE(error);

        for (int i = 0; i < metrics.size(); i++)
        {
            auto param_name = metrics[i].ns().back().value;

            int cpu_id;
            double cmt_data;
            int l3_total_size = 300 * num_cores;
            int l3_num_ways = 200 * num_cores;
            int l3_way_size = 10 * num_cores;
            try {
                cpu_id = std::stoi(metrics[i].ns()[metrics[i].ns().size() - 2].value);
                cmt_data = (100 * (cpu_id + 1));
            } catch(...) {
                cpu_id = -1;
                cmt_data = -1;
            }

            if (param_name.compare("bytes") == 0) {
                EXPECT_EQ(metrics[i].get_int_data(), cmt_data);
            }
            else if (param_name.compare("percentage") == 0) {
                EXPECT_TRUE(metrics[i].get_float64_data() - (cmt_data / l3_total_size) * 100 < 0.000001);
            }
            else if (param_name.compare("cmt_capability") == 0) {
                EXPECT_EQ(metrics[i].get_int_data(), 1);
            }
            else if (param_name.compare("mbm_local_monitoring") == 0) {
                EXPECT_EQ(metrics[i].get_int_data(), 1);
            }
            else if (param_name.compare("cache_allocation") == 0) {
                EXPECT_EQ(metrics[i].get_int_data(), 0);
            }
            else if (param_name.compare("llc_size") == 0) {
                EXPECT_EQ(metrics[i].get_int_data(), l3_total_size);
            }
            else if (param_name.compare("cache_ways_count") == 0) {
                EXPECT_EQ(metrics[i].get_int_data(), l3_num_ways);
            }
            else if (param_name.compare("cache_way_size") == 0) {
                EXPECT_EQ(metrics[i].get_int_data(), l3_way_size);
            }
        }
        EXPECT_CALL(p_mock, pqos_mon_stop(_)).Times(num_cores);
        EXPECT_CALL(p_mock, pqos_fini()).Times(1);
        delete(rdt);
        delete(p_cpu);
    }

    delete(p_cap);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
