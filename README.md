<!--
 Copyright (c) 2017 Intel Corporation

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
-->

# Snap Intel Resource Director Technology Collector

RDT enables Cache Occupancy measurment and Memory Bandwidth monitoring.

For more information about RDT, please see http://www.intel.com/content/www/us/en/architecture-and-technology/resource-director-technology.html

This plugin utilizes https://github.com/01org/intel-cmt-cat.

## Vagrant configuration
Vagrant configuration needed for VirtualBox provider:
```bash
vagrant plugin install vagrant-vbguest
```

## Quick start instructions:
```bash
cd Vagrant
vagrant up
```

## To rebuild plugin after changes done:
```bash
vagrant ssh
cd snap-plugin-collector-rdt/
./build.sh
```

## To build with coverage data and run unit tests:
```bash
vagrant ssh
cd snap-plugin-collector-rdt/
TEST_TYPE=medium ./test.sh
```

