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

#include "medium_test.hpp"
#include "rdt/rdt_utils.hpp"
#include "gtest/gtest.h"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace rdt
{

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgumentPointee;
using ::testing::SaveArgPointee;
using ::testing::Invoke;

#define CMT_METRICS_PER_CORE 2
#define MBM_LOCAL_METRICS_PER_CORE 1
#define MBM_REMOTE_METRICS_PER_CORE 2
#define CAP_METRICS 7

bool is_prefix(std::string const &prefix, std::string const &str);
std::string extract_ns(const Plugin::Metric &metric);
pqos_cap *mock_caps(std::vector<pqos_mon_event> events);
pqos_cpuinfo *mock_cpu(unsigned num_cores);
void mock_mon_poll(struct pqos_mon_data **groups, const unsigned num_groups);
std::vector<Plugin::Metric> fixture_metrics(unsigned cpu_cores);
void test_get_metric_types(PQOSMock *p_mock, pqos_cap *p_cap, unsigned num_cores, int expected_count);
void test_collected_metrics_content(PQOSMock *p_mock, pqos_cap *p_cap, unsigned num_cores);
void test_collect_no_cmt_cap(PQOSMock *p_mock, pqos_cap *p_cap, unsigned num_cores);
void test_collect_err(PQOSMock *p_mock, pqos_cap *p_cap, unsigned num_cores, std::string const &expected_err);
void emplace_per_core_metrics(rdt_metric_map *expected_values, double llc_size, int cpu_id);

// Test PQOS init failure on collector construct
TEST(CollectorContructor, TestPQOSInitFailure)
{
    PQOSMock p_mock;

    // Checking PQOS init failure
    EXPECT_CALL(p_mock, pqos_init(_)).WillOnce(Return(PQOS_RETVAL_ERROR));
    EXPECT_CALL(p_mock, pqos_cap_get(_, _)).Times(0);
    EXPECT_CALL(p_mock, pqos_fini()).Times(0);

    EXPECT_THROW(new rdt::Collector(&p_mock), Plugin::PluginException);
}

// Test get PQOS capabilities failure on collector construct
TEST(CollectorContructor, TestPQOSCapGetFailure)
{
    PQOSMock p_mock;

    // Checking PQOS init failure
    EXPECT_CALL(p_mock, pqos_init(_)).WillOnce(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillOnce(Return(PQOS_RETVAL_ERROR));
    EXPECT_CALL(p_mock, pqos_fini()).Times(0);

    EXPECT_THROW(new rdt::Collector(&p_mock), Plugin::PluginException);
}

// Test successfull collector construct
TEST(CollectorContructor, TestConstructorSuccess)
{
    PQOSMock p_mock;
    pqos_cap *p_cap = mock_caps(std::vector<pqos_mon_event>{});
    pqos_cpuinfo *p_cpu = mock_cpu(1);

    // Checking PQOS init failure
    EXPECT_CALL(p_mock, pqos_init(_)).WillOnce(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_cap_get(_, _)).WillOnce(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));
    EXPECT_CALL(p_mock, pqos_fini()).Times(0);

    EXPECT_NO_THROW(new rdt::Collector(&p_mock));

    delete (p_cpu);
    delete (p_cap);
}

// Test get metric types with CMT capability available
TEST(GetMetricTypesTest, TestMetricCountMultiCpu)
{
    PQOSMock p_mock;
    pqos_cap *p_cap;
    pqos_cpuinfo *p_cpu;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_L3_OCCUP, PQOS_MON_EVENT_LMEM_BW, PQOS_MON_EVENT_RMEM_BW});
    EXPECT_TRUE(rdt::has_cmt_capability(p_cap));
    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    int mts_per_core = CMT_METRICS_PER_CORE + MBM_LOCAL_METRICS_PER_CORE + MBM_REMOTE_METRICS_PER_CORE;
    test_get_metric_types(&p_mock, p_cap, 1, CAP_METRICS + 1 * mts_per_core);
    test_get_metric_types(&p_mock, p_cap, 2, CAP_METRICS + 2 * mts_per_core);
    test_get_metric_types(&p_mock, p_cap, 3, CAP_METRICS + 3 * mts_per_core);
    test_get_metric_types(&p_mock, p_cap, 4, CAP_METRICS + 4 * mts_per_core);

    delete (p_cap);
}

// test_get_metric_types takes capabilities and checks if expected_count of metrics is returned for provided num_cores
void test_get_metric_types(PQOSMock *p_mock, pqos_cap *p_cap, unsigned num_cores, int expected_count)
{
    auto p_cpu = mock_cpu(num_cores);

    EXPECT_CALL(*p_mock, pqos_cap_get(_, _)).WillOnce(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));
    rdt::Collector *rdt = new rdt::Collector(p_mock);
    rpc::ConfigMap map;
    Plugin::Config config(map);
    auto metric_types = rdt->get_metric_types(config);

    EXPECT_EQ(metric_types.size(), expected_count);
    EXPECT_CALL(*p_mock, pqos_fini()).Times(1);
    delete (rdt);
    delete (p_cpu);
}

// Test get metric types without CMT capability available
TEST(GetMetricTypesTest, TestMetricCountMultiCpuNoCmtCap)
{
    PQOSMock p_mock;
    pqos_cap *p_cap;
    pqos_cpuinfo *p_cpu;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_LMEM_BW, PQOS_MON_EVENT_RMEM_BW});
    EXPECT_FALSE(rdt::has_cmt_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    int mts_per_core = MBM_LOCAL_METRICS_PER_CORE + MBM_REMOTE_METRICS_PER_CORE;
    test_get_metric_types(&p_mock, p_cap, 1, CAP_METRICS + 1 * mts_per_core);
    test_get_metric_types(&p_mock, p_cap, 2, CAP_METRICS + 2 * mts_per_core);
    test_get_metric_types(&p_mock, p_cap, 3, CAP_METRICS + 3 * mts_per_core);
    test_get_metric_types(&p_mock, p_cap, 4, CAP_METRICS + 4 * mts_per_core);

    delete (p_cap);
}

// Test get metric types without any RDT capability available
TEST(GetMetricTypesTest, TestMetricCountMultiCpuNoCaps)
{
    PQOSMock p_mock;
    pqos_cap *p_cap;
    pqos_cpuinfo *p_cpu;

    p_cap = mock_caps(std::vector<pqos_mon_event>{});
    EXPECT_FALSE(rdt::has_cmt_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    test_get_metric_types(&p_mock, p_cap, 1, CAP_METRICS);
    test_get_metric_types(&p_mock, p_cap, 2, CAP_METRICS);
    test_get_metric_types(&p_mock, p_cap, 3, CAP_METRICS);
    test_get_metric_types(&p_mock, p_cap, 4, CAP_METRICS);

    delete (p_cap);
}

// Test collecting metrics without CMT capability
TEST(CollectMetricsTest, TestCollectNoCmtCap)
{
    PQOSMock p_mock;
    pqos_cap *p_cap;

    p_cap = mock_caps(std::vector<pqos_mon_event>{});
    EXPECT_FALSE(rdt::has_cmt_capability(p_cap));
    EXPECT_FALSE(rdt::has_local_mbm_capability(p_cap));
    EXPECT_FALSE(rdt::has_remote_mbm_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_reset()).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_start(_, _, _, _, _)).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_stop(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_poll(_, _)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    test_collect_no_cmt_cap(&p_mock, p_cap, 1);
    test_collect_no_cmt_cap(&p_mock, p_cap, 2);
    test_collect_no_cmt_cap(&p_mock, p_cap, 3);
    test_collect_no_cmt_cap(&p_mock, p_cap, 4);

    delete (p_cap);
}

// test_collect_no_cmt_cap takes capabilities and checks if expected_count of metrics is collected for provided num_cores
// This test expects that CMT capabilities are not available
void test_collect_no_cmt_cap(PQOSMock *p_mock, pqos_cap *p_cap, unsigned num_cores)
{
    auto p_cpu = mock_cpu(num_cores);
    EXPECT_CALL(*p_mock, pqos_cap_get(_, _)).WillRepeatedly(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));

    rdt::Collector *rdt = new rdt::Collector(p_mock);

    auto metrics = fixture_metrics(num_cores);
    EXPECT_NO_THROW(rdt->collect_metrics(metrics));

    EXPECT_EQ(metrics.size(), CAP_METRICS);
    EXPECT_CALL(*p_mock, pqos_fini()).Times(1);
    delete (rdt);
    delete (p_cpu);
}

// Test collecting metrics with pqos_mon_start failure
TEST(CollectMetricsTest, TestCollectMonStartErr)
{
    PQOSMock p_mock;
    pqos_cap *p_cap;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_L3_OCCUP});
    EXPECT_TRUE(rdt::has_cmt_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_reset()).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_poll(_, _)).WillRepeatedly(DoAll(Invoke(mock_mon_poll), Return(PQOS_RETVAL_OK)));

    EXPECT_CALL(p_mock, pqos_mon_start(_, _, _, _, _)).WillOnce(Return(PQOS_RETVAL_ERROR));
    test_collect_err(&p_mock, p_cap, 1, "Could not start PQoS monitoring");
    EXPECT_CALL(p_mock, pqos_mon_start(_, _, _, _, _)).WillOnce(Return(PQOS_RETVAL_ERROR));
    test_collect_err(&p_mock, p_cap, 2, "Could not start PQoS monitoring");
    EXPECT_CALL(p_mock, pqos_mon_start(_, _, _, _, _)).WillOnce(Return(PQOS_RETVAL_ERROR));
    test_collect_err(&p_mock, p_cap, 3, "Could not start PQoS monitoring");
    EXPECT_CALL(p_mock, pqos_mon_start(_, _, _, _, _)).WillOnce(Return(PQOS_RETVAL_ERROR));
    test_collect_err(&p_mock, p_cap, 4, "Could not start PQoS monitoring");

    delete (p_cap);
}

// Test collecting metrics with pqos_mon_reset failure
TEST(CollectMetricsTest, TestCollectMonResetErr)
{
    PQOSMock p_mock;
    pqos_cap *p_cap;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_L3_OCCUP});
    EXPECT_TRUE(rdt::has_cmt_capability(p_cap));
    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    EXPECT_CALL(p_mock, pqos_mon_reset()).WillOnce(Return(PQOS_RETVAL_ERROR));
    test_collect_err(&p_mock, p_cap, 1, "Could not reset PQoS RMIDs");
    EXPECT_CALL(p_mock, pqos_mon_reset()).WillOnce(Return(PQOS_RETVAL_ERROR));
    test_collect_err(&p_mock, p_cap, 2, "Could not reset PQoS RMIDs");
    EXPECT_CALL(p_mock, pqos_mon_reset()).WillOnce(Return(PQOS_RETVAL_ERROR));
    test_collect_err(&p_mock, p_cap, 3, "Could not reset PQoS RMIDs");
    EXPECT_CALL(p_mock, pqos_mon_reset()).WillOnce(Return(PQOS_RETVAL_ERROR));
    test_collect_err(&p_mock, p_cap, 4, "Could not reset PQoS RMIDs");

    delete (p_cap);
}

// test_collect_no_cmt_cap takes capabilities and num_cores and checks if expected error message was raised
void test_collect_err(PQOSMock *p_mock, pqos_cap *p_cap, unsigned num_cores, std::string const &expected_err)
{
    auto p_cpu = mock_cpu(num_cores);
    EXPECT_CALL(*p_mock, pqos_cap_get(_, _)).WillRepeatedly(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));

    rdt::Collector *rdt = new rdt::Collector(p_mock);

    auto metrics = fixture_metrics(num_cores);
    EXPECT_THROW(rdt->collect_metrics(metrics), Plugin::PluginException);

    EXPECT_CALL(*p_mock, pqos_fini()).Times(1);
    delete (rdt);

    delete (p_cpu);
}

// Test collecting metrics successfull
TEST(CollectMetricsTest, TestCollectSuccess)
{
    PQOSMock p_mock;
    pqos_cap *p_cap;
    pqos_cpuinfo *p_cpu;

    p_cap = mock_caps(std::vector<pqos_mon_event>{PQOS_MON_EVENT_L3_OCCUP, PQOS_MON_EVENT_RMEM_BW, PQOS_MON_EVENT_LMEM_BW});
    EXPECT_TRUE(rdt::has_cmt_capability(p_cap));

    EXPECT_CALL(p_mock, pqos_init(_)).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_reset()).WillRepeatedly(Return(PQOS_RETVAL_OK));
    EXPECT_CALL(p_mock, pqos_mon_start(_, _, _, _, _)).WillRepeatedly(Return(PQOS_RETVAL_OK));

    test_collected_metrics_content(&p_mock, p_cap, 1);
    test_collected_metrics_content(&p_mock, p_cap, 2);
    test_collected_metrics_content(&p_mock, p_cap, 3);
    test_collected_metrics_content(&p_mock, p_cap, 4);

    delete (p_cap);
}

// test_collected_metrics_content comapres collected data with expected values
void test_collected_metrics_content(PQOSMock *p_mock, pqos_cap *p_cap, unsigned num_cores)
{
    rdt_metric_map expected_values;

    // Expected capabilities values
    const double cache_way_size = 200 * num_cores;
    const double cache_ways_count = 201 * num_cores;
    const double llc_size = 202 * num_cores;

    // Global metrics independent from CPU core number
    expected_values.emplace("/intel/rdt/capabilities/cmt_capability", rdt_metric_data{expected_value : 1, is_float: false});
    expected_values.emplace("/intel/rdt/capabilities/mbm_local_monitoring", rdt_metric_data{expected_value : 1, is_float: false});
    expected_values.emplace("/intel/rdt/capabilities/mbm_remote_monitoring", rdt_metric_data{expected_value : 1, is_float: false});
    expected_values.emplace("/intel/rdt/capabilities/cache_allocation", rdt_metric_data{expected_value : 0, is_float: false});
    expected_values.emplace("/intel/rdt/capabilities/llc_size", rdt_metric_data{expected_value : llc_size, is_float: false});
    expected_values.emplace("/intel/rdt/capabilities/cache_ways_count", rdt_metric_data{expected_value : cache_ways_count, is_float: false});
    expected_values.emplace("/intel/rdt/capabilities/cache_way_size", rdt_metric_data{expected_value : cache_way_size, is_float: false});

    // Mocks
    auto p_cpu = mock_cpu(num_cores);
    EXPECT_CALL(*p_mock, pqos_cap_get(_, _)).WillRepeatedly(DoAll(SetArgumentPointee<0>(p_cap), SetArgumentPointee<1>(p_cpu), Return(PQOS_RETVAL_OK)));
    EXPECT_CALL(*p_mock, pqos_mon_poll(_, _)).WillOnce(DoAll(Invoke(mock_mon_poll), Return(PQOS_RETVAL_OK)));

    // Start collecting all metrics
    rdt::Collector *rdt = new rdt::Collector(p_mock);
    auto metrics = fixture_metrics(num_cores);
    EXPECT_NO_THROW(rdt->collect_metrics(metrics));

    // Fill expected values map with per-core metric values
    for (int cpu_id = 0; cpu_id < num_cores; cpu_id++) {
        emplace_per_core_metrics(&expected_values, llc_size, cpu_id);
    }

    // Checking if collected metrics values equal expected ones
    for (int i = 0; i < metrics.size(); i++)
    {
        auto metric_ns = extract_ns(metrics[i]);
        auto tested_ns = expected_values.find(metric_ns) != expected_values.end();
        EXPECT_TRUE(tested_ns);

        if (tested_ns) {
            if (expected_values[metric_ns].is_float)
                EXPECT_EQ(expected_values[metric_ns].expected_value, metrics[i].get_float64_data());
            else
                EXPECT_EQ(expected_values[metric_ns].expected_value, metrics[i].get_int_data());
        }
    }

    EXPECT_CALL(*p_mock, pqos_mon_stop(_)).Times(num_cores);
    EXPECT_CALL(*p_mock, pqos_fini()).Times(1);
    delete (rdt);
    delete (p_cpu);
}

// emplace_per_core_metrics fills expected metric value map with per-core metrics
void emplace_per_core_metrics(rdt_metric_map *expected_values, double llc_size, int cpu_id) {
    // Expected per-core values
    const double cmt_data = (100 * (cpu_id + 1));
    const double mbm_data_local = (101 * (cpu_id + 1));
    const double mbm_data_remote = (102 * (cpu_id + 1));
    const double mbm_data_total = (103 * (cpu_id + 1));
    const double llc_percentage = (cmt_data / llc_size) * 100;

    // List of per-core metrics and expected values
    auto cpu_id_entry = std::to_string(cpu_id);
    expected_values->emplace("/intel/rdt/llc_occupancy/" + cpu_id_entry + "/bytes", rdt_metric_data{expected_value : cmt_data, is_float: false});
    expected_values->emplace("/intel/rdt/llc_occupancy/" + cpu_id_entry + "/percentage", rdt_metric_data{expected_value : llc_percentage, is_float: true});
    expected_values->emplace("/intel/rdt/memory_bandwidth/local/" + cpu_id_entry + "/bytes", rdt_metric_data{expected_value : mbm_data_local, is_float: false});
    expected_values->emplace("/intel/rdt/memory_bandwidth/remote/" + cpu_id_entry + "/bytes", rdt_metric_data{expected_value : mbm_data_remote, is_float: false});
    expected_values->emplace("/intel/rdt/memory_bandwidth/total/" + cpu_id_entry + "/bytes", rdt_metric_data{expected_value : mbm_data_total, is_float: false});
}

bool is_prefix(std::string const &prefix, std::string const &str)
{
    auto res = std::mismatch(prefix.begin(), prefix.end(), str.begin());
    return res.first == prefix.end();
}

// Extract ns entries and return as single string value
std::string extract_ns(const Plugin::Metric &metric)
{
    std::string ns_str;
    for (auto const &elem : metric.ns())
    {
        ns_str += "/" + elem.value;
    }
    return ns_str;
}

// Mock capabilities object
pqos_cap *mock_caps(std::vector<pqos_mon_event> events)
{
    // Mock capability list
    auto pqosCapMock = new pqos_capability;

    pqosCapMock->type = PQOS_CAP_TYPE_MON;
    pqosCapMock->u.mon = (pqos_cap_mon *)malloc(sizeof(pqos_cap_mon) + sizeof(pqos_monitor) * events.size());
    pqosCapMock->u.mon->num_events = events.size();
    for (int i = 0; i < pqosCapMock->u.mon->num_events; i++)
        pqosCapMock->u.mon->events[i] = {events[i]};

    // Assign capability list to capabilities object
    auto result = (pqos_cap *)malloc(sizeof(pqos_cap) + sizeof(pqos_capability));
    result->num_cap = 1;
    result->capabilities[0] = *pqosCapMock;

    return result;
}

// Mock CPU object
pqos_cpuinfo *mock_cpu(unsigned num_cores)
{
    auto mockCpuInfo = new pqos_cpuinfo;
    mockCpuInfo->num_cores = num_cores;
    mockCpuInfo->l3.way_size = 200 * num_cores;
    mockCpuInfo->l3.num_ways = 201 * num_cores;
    mockCpuInfo->l3.total_size = 202 * num_cores;
    return mockCpuInfo;
}

// Mock monitor group data
void mock_mon_poll(struct pqos_mon_data **groups, const unsigned num_groups)
{
    for (unsigned i = 0; i < num_groups; i++)
    {
        groups[i]->cores = new unsigned(i);
        groups[i]->poll_ctx = new pqos_mon_poll_ctx;
        groups[i]->values.llc = 100 * (i + 1);
        groups[i]->values.mbm_local_delta = 101 * (i + 1);
        groups[i]->values.mbm_remote_delta = 102 * (i + 1);
        groups[i]->values.mbm_total_delta = 103 * (i + 1);
    }
}

// Mock metrics to collect_metrics
std::vector<Plugin::Metric> fixture_metrics(unsigned cpu_cores)
{
    auto metrics = std::vector<Plugin::Metric>{
        Plugin::Metric(rdt::cmt_capability_ns, "bool", "This CPU supports LLC Cache Monitoring."),
        Plugin::Metric(rdt::mbm_local_monitoring_ns, "bool", "This CPU supports Local Memory Bandwidth Monitoring."),
        Plugin::Metric(rdt::llc_size_ns, "bytes", "LLC Size."),
        Plugin::Metric(rdt::cache_ways_count_ns, "bytes", "Number of cache ways in Last Level Cache."),
        Plugin::Metric(rdt::cache_way_size_ns, "bytes", "Size of cache way in Last Level Cache.")};

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
        Plugin::Metric local_membw_usage_bytes(
            {{"intel"}, {"rdt"}, {"memory_bandwidth"}, {"local"}, dynamicCoreIdElement, {"bytes"}},
            "bytes",
            "Local memory bandwidth usage for CPU " + core_id + " in bytes.");
        Plugin::Metric remote_membw_usage_bytes(
            {{"intel"}, {"rdt"}, {"memory_bandwidth"}, {"remote"}, dynamicCoreIdElement, {"bytes"}},
            "bytes",
            "Remote memory bandwidth usage for CPU " + core_id + " in bytes.");
        Plugin::Metric total_membw_usage_bytes(
            {{"intel"}, {"rdt"}, {"memory_bandwidth"}, {"total"}, dynamicCoreIdElement, {"bytes"}},
            "bytes",
            "Total memory bandwidth usage for CPU " + core_id + " in bytes.");

        metrics.push_back(llc_occupancy_bytes);
        metrics.push_back(llc_occupancy_percentage);
        metrics.push_back(local_membw_usage_bytes);
        metrics.push_back(remote_membw_usage_bytes);
        metrics.push_back(total_membw_usage_bytes);
    }

    return metrics;
}

}  // namespace rdt
