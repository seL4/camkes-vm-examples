package main

import (
    "fmt"
    "orchestration/helpers"
    "orchestration/types"
    "orchestration/wireguard"
    // "orchestration/functionality"
    // "orchestration/nodes"
    // "orchestration/network"
    // "orchestration/build"
)

// type FunctionalityInfo = types.FunctionalityInfo
// type NodeInfo = types.NodeInfo
// type DebugInfo  = types.DebugInfo
type Config  = types.Config
// type IPInfo = types.IPInfo




func main() {

    config_file := "/root/config.yml"
    // config_file := "/home/test/capsule-dev/camkes-vm-examples-manifest/projects/vm-examples/apps/Arm/vm_multi/network/config.yml"

    // Read the config file
    var network_settings Config
    var err error

    network_settings, err = helpers.LoadSettings(config_file)
    if err != nil {
        helpers.LogE(err);
        return
    }
    

    dataPathNumber := 0
    for name, settings := range network_settings.DataPaths {
        helpers.LogE("Setting up Data Path:", name)

        // Write wireguard config file
        wireguard.StartWireGuard(settings.Ingest, dataPathNumber, network_settings.Debug)


        // Give each function a unique vlan tag
        // TODO: Add catching vlan tag overflow
        vid_index := 2; // 1 is reserved for management
        for function_name, function_settings := range network_settings.DataPaths[name].Functions {
            if vid_index + 1 >= 4096 { 
                helpers.LogE("Error. Too many vlans required for this configuration. Quitting.");
            }
            new_vids := [2]int{vid_index, vid_index + 1}
            function_settings.VIDS = new_vids
            network_settings.DataPaths[name].Functions[function_name] = function_settings
        }

        // Create connections for all openvswitch pairs in functionality



        // Create IP addresses for ingest nodes




        fmt.Println(settings)


        dataPathNumber++
    }


    // Deploy the data paths (since there are no errors)

    fmt.Println(network_settings)
    
/*
    // Generate an array to hold functionality info and map to go from user name to important info
    functionality_map := make(map[string]*FunctionalityInfo)

    // Iterate over clusters and 
    //   1) Count how many nodes the user is requesting (saved in num_requested_nodes)
    //   2) Collect a list of all the functionality requested and generate a map from the names to 
    //       the actual node info (will be populated with node data later)
    num_requested_nodes := 0;
    for _, cluster := range network_settings.Clusters {
        num_requested_nodes += len(cluster.Nodes)
        // Populate the functionality map with initial info 
        //    (functionality names to functionality info struct, populated with name only)
        functionality_map = functionality.InitializeFunctionalityMapEntries(functionality_map, cluster.Functionality)
    }
    

    // See how many nodes we are connected to by reading the lease file for infranet
    var available_nodes []NodeInfo

    lease_file := "/var/run/dnsmasq-br0.leases"

    available_nodes, err = nodes.GetAvailableNodes(lease_file)
    if err != nil {
        helpers.LogE("Error finding the number of connected nodes.", err, "Exiting")
        return
    }
    if len(available_nodes) == 0 {
        helpers.LogE("No available nodes on the network. Exiting")
        return
    }

    // Get nodes used for functionality and remove them from the available nodes pool
    // Going to merry this to the functionality info at the end. We want full scope of all 
    //   functionality used to make that decision
    var infrastructure_nodes []NodeInfo
    available_nodes, infrastructure_nodes = functionality.AllocateFunctionalNodes(available_nodes, functionality_map)

    // See if we have enough nodes left over to allocate
    if (num_requested_nodes > len(available_nodes)) {
        helpers.LogE("User tried to request more nodes than are currently available. Exiting")
        return
    }

    // Set up /etc/hosts in vm0 with sel4 names for ease of use
    err = build.EtcHosts(available_nodes, infrastructure_nodes)
    if err != nil { return }


    // Generate an empty mapping to go from user defined names to sel4 names and info
    names_to_nodes := make(map[string]*NodeInfo)


    // Map the user defined nodes and connections into structs that we can use to acutally build
    //   network interfaces sel4 & linux understand
    //   This means:
    //     - Mapping user defined nodes (and their names) to existing sel4 nodes
    //     - Generating the from one node to another (and back)
    //       NOTE: You need to allocate a vlan for each direction, or else openvswitch will send
    //         the data back and forth a bunch
    //   The to and from nodes are going to be set up differently, since the to node doesn't have
    //     a node to receive from and the from node doesn't have a node to send to
    //   This code is going to be annoyingly explicit, the compiler will optimize this for us.
    //     I'm ripping this appart in a bit, so I want to be able to adjust it easily
    index := 0;
    for _, cluster := range network_settings.Clusters {



        // Assign user defined names to each of the sel4 nodes
        // Populate the index of user names to sel4 node info
        index, available_nodes, names_to_nodes = nodes.AssignUserNodeNamesToAvailableSel4Nodes(cluster.Nodes, available_nodes, names_to_nodes, index)


        // Set up the require vlan links on all the remote hosts
        var claimed_vids []int;
        var err_b bool;

        // A list of all the vlans that need to be forwarded between
        var vid_connections [][2]int;

        // Used as a temp variable to create VID connection pairs
        var last_vid int;

        // Used to set up /etc/hosts so that names are automatically installed. Its annoying otherwise
        ConnectedIPs := make([][2]IPInfo, 0);


        for _, connection := range cluster.Connections {

            // This is going to allocate an IP address for the node and an exit vid
            claimed_vids, names_to_nodes, functionality_map, last_vid, err_b = 
                network.AllocateFromNodeNetworkInfo(connection.From, cluster, claimed_vids, names_to_nodes, functionality_map, network_settings.Debug) 
            if err_b {
                helpers.LogE("Error allocating from node. Quitting.")
                return
            }

            // Allocate vid for the through connection
            for _, through := range(connection.Through) {
                claimed_vids, names_to_nodes, functionality_map, vid_connections, last_vid, err_b = 
                    network.AllocateThroughNodeNetworkInfo(through, last_vid, connection.Type, cluster, claimed_vids, names_to_nodes, network_settings.Debug, functionality_map, vid_connections)
                if err_b {
                    helpers.LogE("Error allocating through node. Quitting.")
                    return
                }
            }

            // This is going to allocate an IP address for the node and an ingest vid
            claimed_vids, names_to_nodes, functionality_map, vid_connections, last_vid, err_b = 
                network.AllocateToNodeNetworkInfo(connection.To, last_vid, connection.Type, cluster, claimed_vids, names_to_nodes, network_settings.Debug, functionality_map, vid_connections)
            if err_b {
                helpers.LogE("Error allocating from node. Quitting.")
                return
            }

            // Generate the IP addresses that are connected, it makes life a lot easier with /etc/hosts
            ConnectedIPs = network.SetNewIPConnection(connection, names_to_nodes, ConnectedIPs)


        }
 

        if network_settings.Debug.Verbose { helpers.LogE("VLAN connections:", vid_connections) }

        if network_settings.Debug.Verbose { helpers.LogE("IP connections on OVS switch:", ConnectedIPs) }


        // At this point we have a list of all the nodes, with their vlans, a list of all the
        //    functionality and its vlans, all the nodes have both a sel4 and a user defined 
        //    name, all the IP addresses for each vlan subnet assigned to each node, and a 
        //    connection map to connect all the vlans together for proper forwarding
        var functionality_nodes []FunctionalityInfo
        for _, node := range functionality_map {
            functionality_nodes = append(functionality_nodes, *node)
        }

        var finalized_nodes []NodeInfo
        for _, node := range names_to_nodes {
            finalized_nodes = append(finalized_nodes, *node)
        }

        // Map the requested functionality to the available infrastructure
        functionality_nodes = 
            functionality.AssignFunctionalityToInfra(functionality_nodes, infrastructure_nodes)

        if network_settings.Debug.Verbose {
            helpers.LogE("Functionality Nodes: ", functionality_nodes)
            helpers.LogE("User Nodes: ", finalized_nodes)
        }

        err = build.BuildCluster(functionality_nodes, finalized_nodes, vid_connections, ConnectedIPs)
        if err != nil {
            helpers.LogE("Error building cluster", err)
            return
        }
    }
    */
}


/*
    - Read the config file
        - Pop an error if it isn't there
    - Read how many nodes are requested
    - See if we can allocate enough nodes
        - Report to logs if not
        - Exit
    - Assign names to each of the nodes
    - Determine the number of links needed to get the network running
        - This is going to determine the number of vlans we need
            - Need routing 
    - Determine where all the links need to pass through to get forwarded to the output
    - Set up the different links:
        1) Go to start of connection and set up vlan link
        2) Go to first item in through and set up vlan link in and out
        3) Repeat step 2 with all the p
    - Set up the forwarding in the control node
        1) Iterate through the links and set a forwarding path for the start and end
            - Possibly bi-directional. Shouldn't have to adjust anything but flags though
    - Determine the "functional nodes"
    - Determine what functionality is executing on what functional node
    - Set up the functionality programs
        NOTE: These should take the vids as arguments to make life easier
        NOTE: These need to be stand alone binaries / scripts for QoL
            Would be nice if the can execute from the openvswitch node, but we'll get there 
            eventually
    - QED

    NOTE: Need to deal with node names being the same as functionality names

*/

