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

