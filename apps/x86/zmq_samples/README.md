<!--
  Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)

  SPDX-License-Identifier: CC-BY-SA-4.0
-->

# ZeroMQ cross-vm communication example

The `zmq_samples` application demonstrates messaging between VMs using the ZeroMQ messaging library. Each VM contains a VirtNet
ethernet device `eth0` which is connected to the `eth0` devices in the other VMs. Ethernet packets sent from a VM over this
interface will be transmitted to another VM if the correct macaddress is used. Broadcasting is supported which enables ARP to facilitate Macaddress discovery from an IP.

There are three ZeroMQ application scenarios.  They can be found in zmq_samples/src/user:
- Client/server: A client program that connects to a server program over a network connection and performs a series of requests
- Publish/subscribe: A publish program that publishes information to a network address, and a subscribe program that connects to this address and receives published information
- pipeline: 3 programs, a source, worker and sink, that generate, process and acumulate computational tasks.

The CMakeLists.txt file in zmq_samples builds libzmq (likely found in tools/libzmq) for a linux 32bit target environment, builds the above apps, and installs them into the buildroot rootfs using CMake functions declared found in camkes-vm-linux.
An custom init script is installed for each of the VMs that run each of the scenarios on a single VM, and then accross all 3 VMs, before providing the login prompt for each of them.  Currently if all of the scenarios are run correctly, `Finished running all scenarios` will be printed, otherwise `Exited with error` on an error. If one of the VMs appear to have hung, this could indicate an incorrect network address.

Each network interface is configured as follows:

|vm  |IP          |MAC              |IF  |
|----|------------|-----------------|----|
|vm0 | 192.168.1.1|02:00:00:00:AA:01|eth0|
|vm1 | 192.168.1.2|02:00:00:00:AA:02|eth0|
|vm2 | 192.168.1.3|02:00:00:00:AA:03|eth0|

## Checkout and build

See [Setting up your machine](https://docs.sel4.systems/GettingStarted.html#setting-up-your-machine) for instructions on how to set up your host machine.

```
repo init -u https://github.com/sel4/camkes-vm-examples-manifest.git -b tipc-stage

repo sync

mkdir build-zmq

cd build-zmq

../init-build.sh -DCAMKES_VM_APP=zmq_samples

ninja
```
Then it should be able to be run on a haswell-like machine.

The working output is below.

```
Running 3 scenarios on one vm
emul_raw_tx@virtio_net_vswitch.c:146Running client server scenario
 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
message size: 1 [B]
roundtrip count: 10000
average latency: 20.153 [us]
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:ff:0:aa:3. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
Running pub/sub scenario
6
Collecting updates from weather server…
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:16. Dropping frame.
temp: 93901 -25 36 1
temp: 95278 -19 30 2
temp: 52299 34 29 3
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
temp: 94794 33 10 4
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
temp: 65265 16 46 5
Average temperature for zipcode was 7F
Running pipeline scenario
Sending tasks to workers…
Total expected cost: 5021 msec
10.69.81.9.49.:...86..5..13..11..4..77..91.:97..38..74..10..94..89..64..53..15..32.:50..85..34..42..69..emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
emul_raw_tx@virtio_net_vswitch.c:146 Unreachable dest macaddr 33:33:0:0:0:2. Dropping frame.
86..98..79..8..72.:47..89..80..96..74..85..8..85..88..84.:75..84..22..48..94..15..36..57..68..51.:88..17..36..22..58..5..7..56..83..15.:28..30..3..8..25..77..92..32..61..80.:16..35..64..37..83..57..52..19..14..19.:69..2..36..5..23..93..9..30..49..92.:44..76..21..47..83..46..23..74...Total elapsed time: 5364 msec
Running 3 scenarios accross 3 vms
Running client server scenario
[   15.529525] random: crng init done
[   16.555934] random: crng init done
message size: 1 [B]
roundtrip count: 10000
average latency: 87.514 [us]
Collecting updates from weather server…
Running pub/sub scenario
6
temp: 74943 -21 27 1
temp: 37849 -33 59 2
temp: 30536 67 51 3
temp: 98210 -11 30 4
temp: 56436 -39 20 5
Average temperature for zipcode was -7F

Welcome to Buildroot
buildroot login: Running pipeline scenario
Sending tasks to workers…
Total expected cost: 5494 msec

Welcome to Buildroot
buildroot login: 98.42.49.79.40.29.40.32.90.97.63.18.78.25.29.56.84.32.59.46.84.38.4.35.22.57.70.55.95.51.30.48.94.58.72.53.99.83.61.4.90.32.46.58.22.10.33.46.49.75.100.95.:.........:.........:.........:.........:.........39.8.82.88.50.77.66.81.21.36.47.93.50.10.22.52.18.91.63.32.41.79.40.94.60.73.97.18.53.36.78.53.:.........:.........:.........:.........:.........Total elapsed time: 3282 msec
Finished running all scenarios

Welcome to Buildroot
buildroot login:
```

#### Changing connection topology

The topology definition can be found in zmq_samples.camkes.
- To change the macaddress of a vm, `VM[ID]_MACADDRESS` needs to be updated
- To change what vms a vm connects to: the vms need to be removed for each other's `topology_def`
- To create partitioned topologies, a separate topology_def needs to be created and separate calls to each of the macros in the file will need to be added
- For VMs that are in the same topology, it may be possible for vms to read ethernet frames meant for other vms in the same topology.
- If a vm is used in more than one topology it will still only have one network interface, but vms from one topology won't be able to read frames sent over a separate topology.
- It is not currently possible for two vms to be connected to each other in more than one topology.  It is intended that this
  will be changed if a topology becomes tied to a separate network interface in the future.

```
// VM0 is connected to vm1 and vm2
#define VM0_topology_def(f) f(0,1,2)

// VM1 is connected to vm0 and vm2
#define VM1_topology_def(f) f(1,0,2)

// VM2 is connected to vm0 and vm1
#define VM2_topology_def(f) f(2,0,1)

// topology_def is passed into all of the macros below
#define topology_def(f) \
    VM0_topology_def(f) \
    VM1_topology_def(f) \
    VM2_topology_def(f)

// MAC Addresses that each eth0 will have.
#define VM0_MACADDRESS "02:00:00:00:AA:01"
#define VM1_MACADDRESS "02:00:00:00:AA:02"
#define VM2_MACADDRESS "02:00:00:00:AA:03"

component Init0 {
    VM_INIT_DEF()

    // This declares the send and receive interfaces for each connection from vm0
    // This needs to be called in each Init* component that is referred to in the topology_def
    VM_CONNECTION_COMPONENT_DEF(0,topology_def)
}

// A dummy component to serve as the to side of the connection that we create.
component virtqueueinit {
    provides VirtQueue q;
}

// ... Other component declarations

assembly {
    composition {
        // ... The rest of the composition

        // The instance of the dummy component
        component virtqueueinit init;

        // The connection that connects all of the interfaces together.
        VM_CONNECTION_CONNECT_VMS(init.q, topology_def)

    }

    configuration {
        // Specific configuration
        VM_CONNECTION_CONFIG(init.q, topology_def)

        vm0.init_cons = [
            /* Initialise VirtNet eth0 device in vm0 */
            VM_CONNECTION_INIT_HANDLER,
        ];
        // ... The rest of the configuration

    }
}

```

#### Underlying implementation
The underlying mechanisms/interfaces used to implement cross vm communication are:
 - projects/vm/components/VM/configurations/connections.h: Contains the macro API used in zmq_samples.camkes
 - projects/projects_libs/libvirtqueue: Virtqueue implementation that takes buffers from a restricted address space.
 - projects/projects_libs/libvswitch: A library that resolves macaddresses to virtqueues with broadcast support.
 - projects/vm/components/Init/virtio_net_vswitch.c: VirtioNet vmm device.  Uses libvirtqueue, libvswitch and libsel4camkes
   to implement cross component communication
 - projects/vm/templates/seL3VirtQueues-from.template.c: Camkes template for a virtqueue connection end.
 - tools/camkes/libsel4camkes: Provides camkes bindings for libvirtqueue virtqueues backed by Camkes connector objects.

## CakeML Filter Variant

A variant of this app that uses a CakeML component in place of the 3rd VM can
be built by providing an additional option to the build script:

```
$ ../init-build.sh -DCAMKES_VM_APP=zmq_samples -DCAKEML_FILTER=TRUE -DCAKEMLDIR=/path/to/cakeml
```

The CakeML component sits between the two VMs and proxies their traffic to each
other. Presently it doesn't do any filtering.

The following versions of CakeML and HOL are known to work:

* CakeML: 66a35311787bb43f72e8e758209a4745f288cdfe
* CakeML Binary Compiler: 1bb8663a0aeb377eb22aeb600de4bc536e647744
* HOL: 7323105f50960bdec1b33c513576e5d1d313b62f
