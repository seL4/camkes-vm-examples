package build

import (
    "fmt"
    "orchestration/helpers"
    "orchestration/types"
    "os/exec"
    "strconv"
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

func BuildCluster(functionality_nodes []FunctionalityInfo, finalized_nodes []NodeInfo, vid_connections [][2]int) error {

    // Set up forwarding logic in openvswitch host
    for _, conn := range vid_connections {
        fmt.Println("/root/connect-vlans", "--uni", "--vid1", strconv.Itoa(conn[0]), "--vid2", strconv.Itoa(conn[1]), "--br", "br0")
        cmd := exec.Command("/root/connect-vlans", "--uni", "--vid1", strconv.Itoa(conn[0]), "--vid2", strconv.Itoa(conn[1]), "--br", "br0")

        err := cmd.Run()
        if err != nil {
            helpers.LogE("Error creating vlan forwarding path: ", strconv.Itoa(conn[0]) + " to " + strconv.Itoa(conn[1]), err, "Exiting")
            return err
        }
    }


    // Set up the ips and vlans for the user allocated nodes
    for _, node := range finalized_nodes {
        for i := 0; i < len(node.VIDs); i++ {
            cmd := exec.Command("/root/generate-client-vlan", "--node", node.Sel4IP, "--dev", "eth0", "--ip", node.IPs[i], "--vid", strconv.Itoa(node.VIDs[i]))
            fmt.Println("/root/generate-client-vlan", "--node", node.Sel4IP, "--dev", "eth0", "--ip", node.IPs[i], "--vid", strconv.Itoa(node.VIDs[i]))
            err := cmd.Run()
            if err != nil {
                helpers.LogE("Error creating vlan on remote host: ", node.Sel4IP, err, "Exiting")
                return err
            }
        }
    }

    // Set up the functionality nodes
    for _, node := range functionality_nodes {
        for i := 0; i < len(node.VIDs); i += 2 {
            switch node.Name {
                case "silent":
                    cmd := exec.Command("/root/silent", "--uni", "--node", node.Sel4IP, "--vid1", strconv.Itoa(node.VIDs[i]), "--vid2", strconv.Itoa(node.VIDs[i + 1]))
                    err := cmd.Run()
                    if err != nil {
                        helpers.LogE("Error creating silent on remote host: ", node.Sel4IP, err, "Exiting")
                        return err
                    }
                case "silent2":

            }
        }
    }

    return nil
}

