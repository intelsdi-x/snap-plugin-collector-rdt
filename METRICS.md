# Snap Plugin Collector RDT

This plugin has the ability to gather LLC Occupancy, Memory Bandwidth Occupancy and list RDT capabilities of platform.

* LLC Occupancy metric is available from Haswell (Xeon v3) platform.
* Memory Bandwidth metric is available from Broadwell (Xeon v4) platform.
* Capabilities are always returned with metrics.

Namespace | Unit | Description
----------|-----------|-----------|
/intel/rdt/llc_occupancy/[core_id]/bytes                 |               bytes       |    Total LLC Occupancy of CPU in bytes.
/intel/rdt/llc_occupancy/[core_id]/percentage            |               percentage  |    Total LLC Occupancy of CPU in percentage of whole capacity.
/intel/rdt/memory_bandwidth/local/[core_id]/bytes        |               bytes       |    Local memory bandwidth usage for CPU in bytes.
/intel/rdt/memory_bandwidth/remote//bytes                |               bytes       |    Remote (QPI) memory bandwidth usage for CPU in bytes.
/intel/rdt/capabilities/llc_size                         |               bytes       |    LLC Size.
/intel/rdt/capabilities/cache_way_size                   |               bytes       |    Size of cache way in Last Level Cache.
/intel/rdt/capabilities/cache_ways_count                 |               bytes       |    Number of cache ways in Last Level Cache.
/intel/rdt/capabilities/cmt_capability                   |               bool        |    This CPU supports LLC Cache Monitoring.
/intel/rdt/capabilities/mbm_local_monitoring             |               bool        |    This CPU supports Local Memory Bandwidth Monitoring.
/intel/rdt/capabilities/mbm_remote_monitoring            |               bool        |    This CPU supports Remote Memory Bandwidth Monitoring.
