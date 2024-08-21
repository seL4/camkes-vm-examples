package nodes

import (
    "os"
    "bufio"
    "strings"
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


func AssignUserNodeNamesToAvailableSel4Nodes(UserNodeNames []string, available_nodes []NodeInfo, names_to_nodes map[string]*NodeInfo, index int) (int, []NodeInfo, map[string]*NodeInfo) {

    for _, node := range UserNodeNames {
        // Set the user determined name
        available_nodes[index].Name = node
        // Create a mapping from user defined names to sel4 info. Makes building links easier
        names_to_nodes[node] = &available_nodes[index]
        // Move to next node
        index++
    }

    return index, available_nodes, names_to_nodes
}

// Wrapper in case we want to add some logic here that isn't just return all connected nodes
func GetAvailableNodes(filename string) ([]NodeInfo, error) {
    connected, err := get_connected_nodes(filename)
    if err != nil {
        return nil, err
    }
    return connected, nil
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
        nodes = append(nodes, NodeInfo{Sel4Name: parts[3], Mac: parts[1], Sel4IP:parts[2]})
    }

    // Check for errors during scanning
    if err := scanner.Err(); err != nil {
        return nil, err
    }

    // Print the number of lines
    return nodes, nil
}

