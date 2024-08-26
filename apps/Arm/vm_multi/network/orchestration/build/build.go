package build

import (
    "fmt"
    "os"
    "os/exec"
    "strconv"
    "orchestration/helpers"
    "orchestration/types"
)

type FunctionalityInfo = types.FunctionalityInfo
type CrossPathFunctionalityMap = types.CrossPathFunctionalityMap
type Function = types.Function
type NodeInfo = types.NodeInfo
type DebugInfo  = types.DebugInfo
/*
type Cluster  = types.Cluster
type Settings  = types.Settings
type Connection  = types.Connection
type Topology  = types.Topology
type Link  = types.Link
*/
type Config  = types.Config
// type IPInfo = types.IPInfo

func IsolatedDataPath(functionality_nodes map[string]Function, vid_connections [][2]int) error {

    // Set up the functionality nodes
    for name, node := range functionality_nodes {
        fmt.Println("Function node:", name, node)
        switch name {
            case "silent":
                fmt.Println("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[0]), "--vid2", strconv.Itoa(node.VIDs[1]))
                cmd := exec.Command("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[0]), "--vid2", strconv.Itoa(node.VIDs[1]))
                output, err := cmd.CombinedOutput()
                if err != nil {
                    helpers.LogE("Error creating silent on remote host: ", node.Sel4IP, err, string(output), "Exiting")
                    return err
                }
            case "silent2":
                fmt.Println("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[0]), "--vid2", strconv.Itoa(node.VIDs[1]))
                cmd := exec.Command("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[0]), "--vid2", strconv.Itoa(node.VIDs[1]))
                output, err := cmd.CombinedOutput()
                if err != nil {
                    helpers.LogE("Error creating silent on remote host: ", node.Sel4IP, err, string(output), "Exiting")
                    return err
                }
        }
    }


    // Set up forwarding logic in openvswitch host
    for _, conn := range vid_connections {
        fmt.Println("/root/connect-vlans", "--uni", "--vid1", strconv.Itoa(conn[0]), "--vid2", strconv.Itoa(conn[1]), "--br", "br0")
        cmd := exec.Command("/root/connect-vlans", "--uni", "--vid1", strconv.Itoa(conn[0]), "--vid2", strconv.Itoa(conn[1]), "--br", "br0")

        output, err := cmd.CombinedOutput()
        if err != nil {
            helpers.LogE("Error creating vlan forwarding path: ", strconv.Itoa(conn[0]) + " to " + strconv.Itoa(conn[1]), err, string(output), "Exiting")
            return err
        }
    }


    return nil
}

func EtcHosts(available_nodes []NodeInfo) error {

    // Open /etc/hosts for ease of use
    hosts, err := os.OpenFile("/etc/hosts", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
    if err != nil {
        helpers.LogE("Error opening /etc/hosts:", err)
        return err
    }
    defer hosts.Close()

    // Actually write sel4 names to etc hosts
    for _, node := range available_nodes {
        hosts.Write([]byte(node.Sel4IP + " " + node.Sel4Name + "\n"))
    }
    return nil
}

func CrossDataPathNodes(functionality CrossPathFunctionalityMap) error {

    for functionality_type_name, functionality_name_map := range functionality {
        if functionality_type_name == "router" { 
            for router_name, router_settings := range functionality_name_map {
                return BuildCrossDataPathRouter(router_name, router_settings)
            }
        } else {
            return types.NewCrossDataPathFunctionalityError("No known cross data path functionality: " + functionality_type_name)
        }
    }

    return nil
}


func BuildCrossDataPathRouter(router_name string, router_settings Function) error {

    fmt.Println("Cross Data Path Router Settings: ", router_settings)

    for i, dev1 := range router_settings.Interfaces {

        // Get list of all other interfaces we need to connect
        other_interfaces := append(router_settings.Interfaces[:i], router_settings.Interfaces[i+1:]...)
        ip1 := router_settings.IPs[i]

        for j, dev2 := range other_interfaces {
            ip2 := router_settings.IPs[j]
            // Create one path forwarding the internal wireguard interface
            args := []string{"--host", router_settings.Sel4IP, "--dev1", dev1, "--ip1", ip1, "--dev2", dev2, "--ip2", ip2}
            fmt.Println("/root/build-router.sh", args)
            cmd := exec.Command("/root/build-router.sh", args...)
            output, err := cmd.CombinedOutput()
            if err != nil {
                helpers.LogE("Error creating router on remote host: ", router_settings.Name, string(output))
                return err
            }
        }
    } 

    // Create the second path forwarding the internal wireguard interface



    return nil
}

func IngestVlanForwarding(InterfaceName string, ingest_vid int) error {
    
    // Add device to open flow
    args := []string{"add-port", "br0", InterfaceName}
    fmt.Println("ovs-vsctl", args)
    /*
    cmd := exec.Command("ovs-vsctl", args...)
    output, err := cmd.CombinedOutput()
    if err != nil {
        helpers.LogE("Error adding wiregaurd interface as ovs port: ", InterfaceName, string(output))
        return err
    }
    */

    // Set up the vlan forwarding for openflow
    // ovs-ofctl add-flow br0 in_port=InterfaceName,actions=mod_vlan_vid:ingest_vid,output=eth0

    args = []string{"add-flow", "br0", fmt.Sprintf("in_port=%v,actions=mod_vlan_vid:%v,output=eth0", InterfaceName, strconv.Itoa(ingest_vid))}
    fmt.Println("ovs-ofctl", args)
    /*
    cmd = exec.Command("ovs-ofctl", args...)
    output, err = cmd.CombinedOutput()
    if err != nil {
        helpers.LogE("Error adding flow to wiregaurd interface: ", InterfaceName, string(output))
        return err
    }
    */

    return nil
}

