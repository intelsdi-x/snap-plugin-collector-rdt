DISCONTINUATION OF PROJECT. 

This project will no longer be maintained by Intel.

This project has been identified as having known security escapes.

Intel has ceased development and contributions including, but not limited to, maintenance, bug fixes, new releases, or updates, to this project.  

Intel no longer accepts patches to this project.
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


# DISCONTINUATION OF PROJECT 

**This project will no longer be maintained by Intel.  Intel will not provide or guarantee development of or support for this project, including but not limited to, maintenance, bug fixes, new releases or updates.  Patches to this project are no longer accepted by Intel. If you have an ongoing need to use this project, are interested in independently developing it, or would like to maintain patches for the community, please create your own fork of the project.**


# Snap's Intel&#174; Resource Director Technology Collector

[![Build Status](https://travis-ci.com/intelsdi-x/snap-plugin-collector-rdt.svg?token=umXxW83ue2prATx1hZZ9&branch=master)](https://travis-ci.com/intelsdi-x/snap-plugin-collector-rdt)

Intel&#174; RDT enables Cache Occupancy measurment and Memory Bandwidth monitoring. For more information please see http://www.intel.com/content/www/us/en/architecture-and-technology/resource-director-technology.html

This plugin utilizes https://github.com/01org/intel-cmt-cat. It has been tested on CentOS 7.3.1611, Kernel 3.14.32 on Xeon D-1541.

To learn more about Snap Framework, please see [https://github.com/intelsdi-x/snap](https://github.com/intelsdi-x/snap).

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
