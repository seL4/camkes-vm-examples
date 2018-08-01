<!--
     Copyright 2018, Data61
     Commonwealth Scientific and Industrial Research Organisation (CSIRO)
     ABN 41 687 119 230.

     This software may be distributed and modified according to the terms of
     the BSD 2-Clause license. Note that NO WARRANTY is provided.
     See "LICENSE_BSD2.txt" for details.

     @TAG(DATA61_BSD)
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

        vm0_config.init_cons = [
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
