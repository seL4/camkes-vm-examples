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
type NodeInfo = types.NodeInfo
type DebugInfo  = types.DebugInfo
type Cluster  = types.Cluster
type Settings  = types.Settings
type Connection  = types.Connection
type Topology  = types.Topology
type Link  = types.Link
type Config  = types.Config
type IPInfo = types.IPInfo

func BuildCluster(functionality_nodes []FunctionalityInfo, finalized_nodes []NodeInfo, vid_connections [][2]int, ConnectedIPs [][2]IPInfo) error {


    // Set up forwarding logic in openvswitch host
    for _, conn := range vid_connections {
        fmt.Println("/root/connect-vlans", "--uni", "--vid1", strconv.Itoa(conn[0]), "--vid2", strconv.Itoa(conn[1]), "--br", "br0")
        cmd := exec.Command("/root/connect-vlans", "--uni", "--vid1", strconv.Itoa(conn[0]), "--vid2", strconv.Itoa(conn[1]), "--br", "br0")

        output, err := cmd.CombinedOutput()
        if err != nil {
            helpers.LogE("Error creating vlan forwarding path: ", strconv.Itoa(conn[0]) + " to " + strconv.Itoa(conn[1]), err, output, "Exiting")
            return err
        }
    }


    // Set up the ips and vlans for the user allocated nodes
    for _, node := range finalized_nodes {
        for i := 0; i < len(node.VIDs); i++ {
            cmd := exec.Command("/root/generate-client-vlan", "--node", node.Sel4IP, "--dev", "eth0", "--ip", node.IPs[i], "--vid", strconv.Itoa(node.VIDs[i]))
            fmt.Println("/root/generate-client-vlan", "--node", node.Sel4IP, "--dev", "eth0", "--ip", node.IPs[i], "--vid", strconv.Itoa(node.VIDs[i]))
            output, err := cmd.CombinedOutput()
            if err != nil {
                helpers.LogE("Error creating vlan on remote host: ", node.Sel4IP, err, output, "Exiting")
                return err
            }
        }
    }


    // Set up the functionality nodes
    for _, node := range functionality_nodes {
        for i := 0; i < len(node.VIDs); i += 2 {
            switch node.Name {
                case "silent":
                    fmt.Println("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[i]), "--vid2", strconv.Itoa(node.VIDs[i + 1]))
                    cmd := exec.Command("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[i]), "--vid2", strconv.Itoa(node.VIDs[i + 1]))
                    output, err := cmd.CombinedOutput()
                    if err != nil {
                        helpers.LogE("Error creating silent on remote host: ", node.Sel4IP, err, output, "Exiting")
                        return err
                    }
                case "silent2":
                    fmt.Println("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[i]), "--vid2", strconv.Itoa(node.VIDs[i + 1]))
                    cmd := exec.Command("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[i]), "--vid2", strconv.Itoa(node.VIDs[i + 1]))
                    output, err := cmd.CombinedOutput()
                    if err != nil {
                        helpers.LogE("Error creating silent on remote host: ", node.Sel4IP, err, output, "Exiting")
                        return err
                    }
            }
        }
    }


    // Set up /etc/hosts on remote machine (in shell script for ssh reasons)
    for _, ip_conn := range ConnectedIPs {
        // Set up on the To node
        fmt.Println("/root/remote-etc-hosts", "--remote", ip_conn[0].Sel4Name, "--host", ip_conn[1].UserName, "--ip", ip_conn[1].UserIP)
        cmd := exec.Command("/root/remote-etc-hosts", "--remote", ip_conn[0].Sel4Name, "--host", ip_conn[1].UserName, "--ip", ip_conn[1].UserIP)

        output, err := cmd.CombinedOutput()
        if err != nil {
            helpers.LogE("Error creating /etc/hosts on remote host: ", output, " Exiting")
            return err
        }

        // Set up on the From node
        fmt.Println("/root/remote-etc-hosts", "--remote", ip_conn[1].Sel4Name, "--host", ip_conn[0].UserName, "--ip", ip_conn[0].UserIP)
        cmd = exec.Command("/root/remote-etc-hosts", "--remote", ip_conn[1].Sel4Name, "--host", ip_conn[0].UserName, "--ip", ip_conn[0].UserIP)

        output, err = cmd.CombinedOutput()
        if err != nil {
            helpers.LogE("Error creating /etc/hosts on remote host.", output, "Exiting")
            return err
        }

    }

    return nil
}

func EtcHosts(available_nodes []NodeInfo, infrastructure_nodes []NodeInfo) error {

    // Open /etc/hosts for ease of use
    hosts, err := os.OpenFile("/etc/hosts", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
    if err != nil {
        fmt.Println("Error opening /etc/hosts:", err)
        return err
    }
    defer hosts.Close()

    // Actually write sel4 names to etc hosts
    for _, node := range available_nodes {
        hosts.Write([]byte(node.Sel4IP + " " + node.Sel4Name + "\n"))
    }
    for _, node := range infrastructure_nodes {
        hosts.Write([]byte(node.Sel4IP + " " + node.Sel4Name + "\n"))
    }
    return nil
}

