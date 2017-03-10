# snap-plugin-collector-rdt
RDT Collector for Snap

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
./test.sh
```
