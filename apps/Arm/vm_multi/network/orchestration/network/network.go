package network

import (
    "orchestration/helpers"
    "orchestration/types"
    "math/rand"
)

type FunctionalityInfo = types.FunctionalityInfo
type NodeInfo = types.NodeInfo
type DebugInfo  = types.DebugInfo
type Cluster  = types.Cluster
type Settings  = types.Settings
type Connection  = types.Connection
type Topology  = types.Topology
type Link  = types.Link type Config  = types.Config
type IPInfo = types.IPInfo


func generate_vid(claimed_vids []int, debug_info DebugInfo) ([]int, int) {

    // Pop error if we've hit limit on vlan number
    if len(claimed_vids) == 4096 { return claimed_vids, -1 }

    var vid int;

    // Generate ordered vid if debug is specified
    if debug_info.OrderVlans { 
        vid = len(claimed_vids) + 2;
    } else { // Generate random vid to keep annonymous

        // Generate a random unused vid
        vid := rand.Intn(4094) + 2
        for helpers.InArray(vid, claimed_vids) { vid = rand.Intn(4094) + 2 }

    }

    // Save the new vid
    claimed_vids = append(claimed_vids, vid)

    return claimed_vids, vid

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


func AllocateVid(index string, cluster Cluster, claimed_vids []int, names_to_nodes map[string]*NodeInfo, debug_info DebugInfo, functionality_map map[string]*FunctionalityInfo) ([]int, map[string]*NodeInfo, map[string]*FunctionalityInfo, bool) {
    var err_b bool;

    if helpers.InArray(index, cluster.Nodes) {
        claimed_vids, names_to_nodes, err_b = allocate_node_vid(index, claimed_vids, debug_info, names_to_nodes)
        if err_b {
            helpers.LogE("Error allocating VID for node. Exiting");
            return claimed_vids, names_to_nodes, functionality_map, true
        }
    } else if helpers.InArray(index, cluster.Functionality) { 
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


func AllocateIP(userName string, sel4Name string, vid int, names_to_nodes map[string]*NodeInfo, claimed_vids []int) (map[string]*NodeInfo, bool) {
    if userName == "external" { return names_to_nodes, false; }
    // Assign IPs to the from node. Use vlanid to create unique subnets. EZPZ
    if len(names_to_nodes[userName].Sel4Name) < 3 {
        helpers.LogE("Malformated device name in leases:", names_to_nodes[userName].Sel4Name)
        return names_to_nodes, true
    }

    // Uses claimed VIDs to prevent possible IP collisions
    ip := helpers.GenerateIp(names_to_nodes[userName].Sel4Name, claimed_vids[len(claimed_vids) - 1])
    names_to_nodes[userName].IPs = append(names_to_nodes[userName].IPs, ip)

    return names_to_nodes, false
}


func AllocateFromNodeNetworkInfo(FromName string, cluster Cluster, claimed_vids []int, names_to_nodes map[string]*NodeInfo, functionality_map map[string]*FunctionalityInfo, debug_info DebugInfo) ([]int, map[string]*NodeInfo, map[string]*FunctionalityInfo, int, bool) {

    var err_b bool;

    // Allocate vid for the from connection and label its directionality
    claimed_vids, names_to_nodes, functionality_map, err_b = 
        AllocateVid(FromName, cluster, claimed_vids, names_to_nodes, debug_info, functionality_map)
    if err_b {
        helpers.LogE("Error allocating VID for the from connection. Exiting");
        return claimed_vids, names_to_nodes, functionality_map, -1, err_b
    }
    // Allocate the IP for the from connection on this particular vlan
    names_to_nodes, err_b = 
        AllocateIP(FromName, names_to_nodes[FromName].Sel4Name, claimed_vids[len(claimed_vids) - 1], names_to_nodes, claimed_vids)
    if err_b {
        helpers.LogE("Error allocating IP for the from connection. Exiting");
        return claimed_vids, names_to_nodes, functionality_map, -1, err_b
    }


    // Set up for through loop
    last_vid := claimed_vids[len(claimed_vids) - 1]

    return claimed_vids, names_to_nodes, functionality_map, last_vid, false
}

func AllocateThroughNodeNetworkInfo(through string, last_vid int, connection_type string, cluster Cluster, claimed_vids []int, names_to_nodes map[string]*NodeInfo, debug_info DebugInfo, functionality_map map[string]*FunctionalityInfo, vid_connections [][2]int) ([]int, map[string]*NodeInfo, map[string]*FunctionalityInfo, [][2]int, int, bool) {

    var err_b bool;

    // Allocate the injest section
    claimed_vids, names_to_nodes, functionality_map, err_b = AllocateVid(through, cluster, claimed_vids, names_to_nodes, debug_info, functionality_map)
    if err_b {
        helpers.LogE("Error allocating VID for a through connection. Exiting");
        return claimed_vids, names_to_nodes, functionality_map, vid_connections, last_vid, true
    }
    // Build connections map for openvswitch connections to be built off of
    vid_connections = append(vid_connections, [2]int{ last_vid, claimed_vids[len(claimed_vids) - 1] })
    // Add reverse direction if bi-directional
    if connection_type != "unidirectional" { 
        vid_connections = append(vid_connections, [2]int{claimed_vids[len(claimed_vids) - 1], last_vid }) 
    }
    // Iterate forward
    last_vid = claimed_vids[len(claimed_vids) - 1]


    // Allocate the exit section
    claimed_vids, names_to_nodes, functionality_map, err_b = AllocateVid(through, cluster, claimed_vids, names_to_nodes, debug_info, functionality_map)
    if err_b {
        helpers.LogE("Error allocating VID for a through connection. Exiting");
        return claimed_vids, names_to_nodes, functionality_map, vid_connections, last_vid, true
    }
    // Build connections map for openvswitch connections to be built off of
    // Don't add openvswitch forwarding map here. This section is for the node to forward
    // vid_connections = append(vid_connections, [2]int{ last_vid, claimed_vids[len(claimed_vids) - 1] })
    // Iterate forward
    last_vid = claimed_vids[len(claimed_vids) - 1]


    return claimed_vids, names_to_nodes, functionality_map, vid_connections, last_vid, false
}

func AllocateToNodeNetworkInfo(To string, last_vid int, connection_type string, cluster Cluster, claimed_vids []int, names_to_nodes map[string]*NodeInfo, debug_info DebugInfo, functionality_map map[string]*FunctionalityInfo, vid_connections [][2]int) ([]int, map[string]*NodeInfo, map[string]*FunctionalityInfo, [][2]int, int, bool) {

    var err_b bool;

    // Allocate vid for the to connection
    claimed_vids, names_to_nodes, functionality_map, err_b = AllocateVid(To, cluster, claimed_vids, names_to_nodes, debug_info, functionality_map)
    if err_b {
        helpers.LogE("Error allocating VID for the from connection. Exiting");
        return claimed_vids, names_to_nodes, functionality_map, vid_connections, last_vid, err_b
    }
    // Add new vid
    vid_connections = append(vid_connections, [2]int{ last_vid, claimed_vids[len(claimed_vids) - 1] })
    // Add the reverse path in openvswitch if bi-directional
    if connection_type != "unidirectional" { 
        vid_connections = append(vid_connections, [2]int{ claimed_vids[len(claimed_vids) - 1], last_vid }) 
    }
    // Iterate forward
    last_vid = claimed_vids[len(claimed_vids) - 1]
    // Assign IPs to the from node. Use vlanid to create unique subnets. EZPZ
    if To != "external" {
        ip := helpers.GenerateIp(names_to_nodes[To].Sel4Name, claimed_vids[len(claimed_vids) - 1])
        names_to_nodes[To].IPs = append(names_to_nodes[To].IPs, ip)
    }

    return claimed_vids, names_to_nodes, functionality_map, vid_connections, last_vid, false
}

func SetNewIPConnection(connection Connection, names_to_nodes map[string]*NodeInfo, ConnectedIPs[][2]IPInfo) [][2]IPInfo {

    var from_info IPInfo;
    var to_info IPInfo;

    // External and other possible functionality will pop an error, just pass
    if fromConnection, exists := names_to_nodes[connection.From]; exists {
        // Create from IP info
        userFromIp := fromConnection.IPs[len(fromConnection.IPs) - 1]
        // Strip off the mask
        userFromIp = userFromIp[:len(userFromIp) - 3]
        // Used for SSH ID
        sel4FromName := fromConnection.Sel4Name
        from_info = IPInfo{UserName: connection.From, UserIP: userFromIp, Sel4Name: sel4FromName}
    }


    if toConnection, exists := names_to_nodes[connection.To]; exists {
        // Create from IP info
        userToIp := toConnection.IPs[len(toConnection.IPs) - 1]
        // Strip off the mask
        userToIp = userToIp[:len(userToIp) - 3]
        // Used for SSH ID
        sel4ToName := toConnection.Sel4Name
        to_info = IPInfo{UserName: connection.To, UserIP: userToIp, Sel4Name: sel4ToName}
    }

    // Add this connection into ConnectedIPs, if its an internal connection
    if from_info.UserName != "" && to_info.UserName != "" {
        ConnectedIPs = append(ConnectedIPs, [2]IPInfo{from_info, to_info})
    }

    return ConnectedIPs
}

