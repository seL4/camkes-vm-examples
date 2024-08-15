package functionality

import (
    "orchestration/types"
    // "orchestration/helpers" // Used to get functionality from functionality_map
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


// Populate the functionality map with initial info (functionality names only)
func InitializeFunctionalityMapEntries(functionality_map map[string]*FunctionalityInfo, functionality [] string) map[string]*FunctionalityInfo {

    for _, el := range functionality { 
        if _, ok := functionality_map[el]; ok { continue; }
        functionality_map[el] = &FunctionalityInfo{Name: el}
    }

    return functionality_map
}


func AllocateFunctionalNodes(available_nodes []NodeInfo, functionality_map map[string]*FunctionalityInfo) ([]NodeInfo, []NodeInfo) {

    // requested_functionality := helpers.GetKeys(functionality_map)

    var functional_nodes []NodeInfo

    functional_nodes = []NodeInfo{ available_nodes[0] }
    available_nodes = available_nodes[1:]


    return available_nodes, functional_nodes
}

func AssignFunctionalityToInfra(functionality_nodes []FunctionalityInfo, infrastructure_nodes []NodeInfo) []FunctionalityInfo {
    for i := 0; i < len(functionality_nodes); i++ {
        functionality_nodes[i].Mac = infrastructure_nodes[0].Mac
        functionality_nodes[i].Sel4Name = infrastructure_nodes[0].Sel4Name
        functionality_nodes[i].Sel4IP = infrastructure_nodes[0].Sel4IP
    }
    return functionality_nodes
}
