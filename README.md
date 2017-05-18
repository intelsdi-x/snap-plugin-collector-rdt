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

This plugin utilizes https://github.com/01org/intel-cmt-cat. It has been tested on CentOS 7.3.1611, Kernel 3.14.32 on Xeon D-1541.

Collected metrics are available in [METRICS.md](METRICS.md).

## Build Instructions on Centos 7
```bash
sudo yum install -y git cmake mc tmux autoconf automake libtool curl make unzip wget clang gcc-c++
git submodule update --init
./install_deps_centos.sh
./build.sh
```

### Vagrant VM

Plugin provides vagrant development environment.
`vagrant-vbguest` plugin is required for directory sync.

```bash
vagrant plugin install vagrant-vbguest
cd Vagrant
vagrant up
```


## To run unit tests with coverage data:

```bash
vagrant ssh
cd snap-plugin-collector-rdt/
TEST_TYPE=medium ./test.sh
```
