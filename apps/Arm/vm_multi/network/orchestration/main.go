package main

import (
    "fmt"
    "gopkg.in/yaml.v3"
    "os"
    "io"
    "orchestration/helpers"
    "orchestration/build"
    "orchestration/types"
    "bufio"
    "strings"
    "math/rand"
)

type FunctionalityInfo = types.FunctionalityInfo
type NodeInfo = types.NodeInfo
type DebugInfo  = types.DebugInfo
type Cluster  = types.Cluster
type Settings  = types.Settings
type Connection  = types.Connection
type Topology  = types.Topology
type Link  = types.Link
type Config  = types.Config



func load_settings(filename string) (Config, error) {
    file, err := os.Open(filename)
    defer file.Close()
    if err != nil {
        helpers.LogE("Error opening config file:", err)
        return Config{}, err
    }

    var parsed_yaml Config

    decoder := yaml.NewDecoder(file)
    decoder.Decode(&parsed_yaml)
    
    if err = decoder.Decode(parsed_yaml); err != nil && err != io.EOF {
        helpers.LogE("error decoding YAML: ", err)
        return Config{}, err
    }


    return parsed_yaml, nil
}


func get_connected_nodes(filename string) ([]NodeInfo, error) {
    file, err := os.Open(filename)
    defer file.Close()
    if err != nil {
        return nil, err
    }

    // Create a scanner to read the file line by line
    scanner := bufio.NewScanner(file)

    var nodes []NodeInfo;

    // Loop through the file line by line
    for scanner.Scan() {
        parts := strings.Split(scanner.Text(), " ")
        nodes = append(nodes, NodeInfo{Sel4Name: "vm" + parts[2][len(parts[2]) - 3:], Mac: parts[1], Sel4IP:parts[2]})
    }

    // Check for errors during scanning
    if err := scanner.Err(); err != nil {
        return nil, err
    }

    // Print the number of lines
    return nodes, nil
}


func get_available_nodes(filename string) ([]NodeInfo, error) {
    connected, err := get_connected_nodes(filename)
    if err != nil {
        return nil, err
    }
    return connected, nil
}

func in_array[T comparable](el T, el_array []T) bool {
    for _, array_el := range(el_array) {
        if array_el == el { return true }
    }
    return false
}

func generate_vid(claimed_vids []int, debug_info DebugInfo) ([]int, int) {

    // Pop error if we've hit limit on vlan number
    if len(claimed_vids) == 4096 { return claimed_vids, -1 }

    var vid int;

    // Generate ordered vid if debug is specified
    if debug_info.OrderVlans { 
        vid = len(claimed_vids);
    } else { // Generate random vid to keep annonymous

        // Generate a random unused vid
        vid := rand.Intn(4096)
        for in_array(vid, claimed_vids) { vid = rand.Intn(4096) }

    }

    // Save the new vid
    claimed_vids = append(claimed_vids, vid)

    return claimed_vids, vid

} 


func allocate_node_vid(index string, claimed_vids []int, debug_info DebugInfo, names_to_nodes map[string]*NodeInfo) ([]int, map[string]*NodeInfo, bool) {

    var vid int
    
    claimed_vids, vid = generate_vid(claimed_vids, debug_info)
    if vid == -1 {
        helpers.LogE("All vids allocated. Unexpected case. Exiting")
        return claimed_vids, names_to_nodes, true
    }
    node := names_to_nodes[index]
    node.VIDs = append(node.VIDs, vid)

    return claimed_vids, names_to_nodes, false
}

func allocate_functionality_vid(index string, claimed_vids []int, debug_info DebugInfo, functionality_map map[string]*FunctionalityInfo) ([]int, map[string]*FunctionalityInfo, bool) {

    var vid int
    var node *FunctionalityInfo
    
    claimed_vids, vid = generate_vid(claimed_vids, debug_info)
    if vid == -1 {
        helpers.LogE("All vids allocated. Unexpected case. Exiting")
        return claimed_vids, functionality_map, true
    }
    node = functionality_map[index]
    node.VIDs = append(node.VIDs, vid)

    return claimed_vids, functionality_map, false
}

func allocate_vid(index string, cluster Cluster, claimed_vids []int, names_to_nodes map[string]*NodeInfo, debug_info DebugInfo, functionality_map map[string]*FunctionalityInfo) ([]int, map[string]*NodeInfo, map[string]*FunctionalityInfo, bool) {
    var err_b bool;

    if in_array(index, cluster.Nodes) {
        claimed_vids, names_to_nodes, err_b = allocate_node_vid(index, claimed_vids, debug_info, names_to_nodes)
        if err_b {
            helpers.LogE("Error allocating VID for node. Exiting");
            return claimed_vids, names_to_nodes, functionality_map, true
        }
    } else if (in_array(index, cluster.Functionality)) { 
        claimed_vids, functionality_map, err_b = allocate_functionality_vid(index, claimed_vids, debug_info, functionality_map);
        if err_b {
            helpers.LogE("Error allocating VID for functionality. Exiting");
            return claimed_vids, names_to_nodes, functionality_map, true
        }
    } else { 
        helpers.LogE("Node specified is not user defined node or functionality. ", "User requested:", index, "Exiting")
        helpers.LogE("Requested nodes are:", cluster.Nodes)
        helpers.LogE("Requested functions are:", cluster.Functionality)
        return claimed_vids, names_to_nodes, functionality_map, true
    }

    return claimed_vids, names_to_nodes, functionality_map, false
}


func generate_ip(sel4Name string, vid int) string {
    upper8 := int((vid >> 8) & 0xFF)
    lower8 := int(vid & 0xFF)
    ip := "10." + fmt.Sprintf("%d", upper8) + "." + fmt.Sprintf("%d", lower8) + "." + sel4Name[2:] + "/24"
    return ip
}




func allocate_functional_nodes(available_nodes []NodeInfo, requested_functionality []string) ([]NodeInfo, []NodeInfo) {

    var functional_nodes []NodeInfo

    functional_nodes = []NodeInfo{ available_nodes[0] }
    available_nodes = available_nodes[1:]


    return available_nodes, functional_nodes
}



func assign_functionality_to_infrastructure(functionality_nodes []FunctionalityInfo, infrastructure_nodes []NodeInfo) []FunctionalityInfo {
    for i := 0; i < len(functionality_nodes); i++ {
        functionality_nodes[i].Mac = infrastructure_nodes[0].Mac
        functionality_nodes[i].Sel4Name = infrastructure_nodes[0].Sel4Name
        functionality_nodes[i].Sel4IP = infrastructure_nodes[0].Sel4IP
    }
    return functionality_nodes
}


func main() {

    config_file := "/root/config.yml"

    // Read the config file
    var network_settings Config
    var err error
    network_settings, err = load_settings(config_file)
    if err != nil {
        helpers.LogE(err);
        return
    }
    

    // See how many nodes the user is requesting
    num_requested_nodes := 0;
    var requested_functionality []string;
    for _, cluster := range network_settings.Clusters {
        num_requested_nodes += len(cluster.Nodes)
        // Collect all the requested functionality in the clusters
        for _, function := range cluster.Functionality {
            if !in_array(function, requested_functionality) { 
                requested_functionality = append(requested_functionality, function)
            }
        }
    }
    


    // See how many nodes we are connected to
    // Read the lease file for all connected nodes
    var available_nodes []NodeInfo
    lease_file := "/var/run/dnsmasq-eth0.leases"
    available_nodes, err = get_available_nodes(lease_file)
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
    available_nodes, infrastructure_nodes = allocate_functional_nodes(available_nodes, requested_functionality)

    // See if we have enough nodes to allocate
    if (num_requested_nodes > len(available_nodes)) {
        helpers.LogE("User tried to request more nodes than are currently available. Exiting")
        return
    }




    // vid to directionality map
    // vid_directionality := make(map[int]string)

    // Generate an empty mapping to go from user defined names to sel4 names
    names_to_nodes := make(map[string]*NodeInfo)


    // Generate an array to hold functionality info and map to go from user name to important info
    functionality_map := make(map[string]*FunctionalityInfo)

    // We could do this above, but I'm being explicit until we need the performance win
    index := 0;
    for _, cluster := range network_settings.Clusters {


        for _, el := range cluster.Functionality { 
            if _, ok := functionality_map[el]; ok { continue; }
            // functionality_nodes = append(functionality_nodes, )
            functionality_map[el] = &FunctionalityInfo{Name: el}
        }

        // Assign names to each of the nodes
        for _, node := range cluster.Nodes {
            // Set the user determined name
            available_nodes[index].Name = node
            // Create a mapping from user defined names to sel4 info. Makes building links easier
            names_to_nodes[node] = &available_nodes[index]
            // Move to next node
            index++
        }

        // Set up the require vlan links on all the remote hosts
        var claimed_vids []int;
        var err_b bool;

        var vid_connections [][2]int;

        var ip string;

        for _, connection := range cluster.Connections {


            // Allocate vid for the from connection and label its directionality
            claimed_vids, names_to_nodes, functionality_map, err_b = allocate_vid(connection.From, cluster, claimed_vids, names_to_nodes, network_settings.Debug, functionality_map)
            if err_b {
                helpers.LogE("Error allocating VID for the from connection. Exiting");
                return
            }
            if connection.From != "external" {
                // Assign IPs to the from node. Use vlanid to create unique subnets. EZPZ
                if len(names_to_nodes[connection.From].Sel4Name) < 3 {
                    helpers.LogE("Malformated device name in leases:", names_to_nodes[connection.From].Sel4Name)
                }
                ip = generate_ip(names_to_nodes[connection.From].Sel4Name, claimed_vids[len(claimed_vids) - 1])
                names_to_nodes[connection.From].IPs = append(names_to_nodes[connection.From].IPs, ip)
            }

            // Set up for through loop
            last_vid := claimed_vids[len(claimed_vids) - 1]


            // Allocate vid for the through connection
            for _, through := range(connection.Through) {
                // Allocate the injest section
                claimed_vids, names_to_nodes, functionality_map, err_b = allocate_vid(through, cluster, claimed_vids, names_to_nodes, network_settings.Debug, functionality_map)
                if err_b {
                    helpers.LogE("Error allocating VID for a through connection. Exiting");
                    return

                }
                // Build connections map for openvswitch connections to be built off of
                vid_connections = append(vid_connections, [2]int{ last_vid, claimed_vids[len(claimed_vids) - 1] })
                // Add reverse direction if bi-directional
                if connection.Type != "unidirectional" { vid_connections = append(vid_connections, [2]int{claimed_vids[len(claimed_vids) - 1], last_vid }) }
                // Iterate forward
                last_vid = claimed_vids[len(claimed_vids) - 1]


                // Allocate the exit section
                claimed_vids, names_to_nodes, functionality_map, err_b = allocate_vid(through, cluster, claimed_vids, names_to_nodes, network_settings.Debug, functionality_map)
                if err_b {
                    helpers.LogE("Error allocating VID for a through connection. Exiting");
                    return

                }
                // Build connections map for openvswitch connections to be built off of
                // Don't add openvswitch forwarding map here. This section is for the node to forward
                // vid_connections = append(vid_connections, [2]int{ last_vid, claimed_vids[len(claimed_vids) - 1] })
                // Iterate forward
                last_vid = claimed_vids[len(claimed_vids) - 1]


            }

            // Allocate vid for the to connection
            claimed_vids, names_to_nodes, functionality_map, err_b = allocate_vid(connection.To, cluster, claimed_vids, names_to_nodes, network_settings.Debug, functionality_map)
            if err_b {
                helpers.LogE("Error allocating VID for the from connection. Exiting");
                return
            }
            // Add new vid
            vid_connections = append(vid_connections, [2]int{ last_vid, claimed_vids[len(claimed_vids) - 1] })
            // Add the reverse path in openvswitch if bi-directional
            if connection.Type != "unidirectional" { vid_connections = append(vid_connections, [2]int{ claimed_vids[len(claimed_vids) - 1], last_vid }) }
            // Iterate forward
            last_vid = claimed_vids[len(claimed_vids) - 1]
            // Assign IPs to the from node. Use vlanid to create unique subnets. EZPZ
            if connection.To != "external" {
                ip = generate_ip(names_to_nodes[connection.To].Sel4Name, claimed_vids[len(claimed_vids) - 1])
                names_to_nodes[connection.To].IPs = append(names_to_nodes[connection.To].IPs, ip)
            }


        }

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
        functionality_nodes = assign_functionality_to_infrastructure(functionality_nodes, infrastructure_nodes, )

        fmt.Println(functionality_nodes)
        fmt.Println(finalized_nodes)

        err = build.BuildCluster(functionality_nodes, finalized_nodes, vid_connections)
        if err != nil {
            helpers.LogE("Error building cluster", err)
            return
        }

    }

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

