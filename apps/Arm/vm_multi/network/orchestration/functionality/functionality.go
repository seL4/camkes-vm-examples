package functionality

import (
    "fmt"
    "orchestration/types"
    // "orchestration/helpers" // Used to get functionality from functionality_map
)

type FunctionalityInfo = types.FunctionalityInfo
type NodeInfo = types.NodeInfo
type DebugInfo  = types.DebugInfo
// type Cluster  = types.Cluster
// type Settings  = types.Settings
// type Connection  = types.Connection
// type Topology  = types.Topology
// type Link  = types.Link
type Config  = types.Config
type Function = types.Function
type DataPath = types.DataPath
type YamlParseError = types.YamlParseError
type CrossPathFunctionalityMap = types.CrossPathFunctionalityMap

/*
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
*/

func RequiresCrossPathAbility(functionality string) bool {
    if functionality == "router" { return true }
    return false
}

var counter int; // Auto zero

func AddCrossPathFunctionality(network_settings Config, function_name string, functionality Function, data_path DataPath, available_nodes []NodeInfo, new_vids []int) (Config, error) {

    if network_settings.CrossPathFunctionality == nil {
        network_settings.CrossPathFunctionality = make(CrossPathFunctionalityMap)
    }

    if _, exists := network_settings.CrossPathFunctionality[function_name]; !exists { 
        network_settings.CrossPathFunctionality[function_name] = make(map[string]Function)
    }

    if _, exists := functionality.Args["name"]; !exists { 
        return network_settings, types.NewYamlParseError(fmt.Sprintf("No 'name' attribute specified in cross path functionality %v", function_name))
    }

    if _, exists := network_settings.CrossPathFunctionality[function_name][functionality.Args["name"]]; !exists {
        network_settings.CrossPathFunctionality[function_name][functionality.Args["name"]] = Function{Name: functionality.Args["name"], Interfaces: []string{}};
    }

    if function_name == "router" {
        new_info := network_settings.CrossPathFunctionality[function_name][functionality.Args["name"]]
        new_info.Interfaces = append(new_info.Interfaces, data_path.Ingest.InterfaceName) // Add this address to the list of known routes
        new_info.IPs = append(new_info.IPs , data_path.Ingest.Address) // Add this address to the list of known routes

        // Assign a node to that functionality
        new_info.Sel4Name = available_nodes[counter].Sel4Name
        new_info.Mac = available_nodes[counter].Mac
        new_info.Sel4IP = available_nodes[counter].Sel4IP
        new_info.VIDs = append(new_info.VIDs, new_vids...)
        counter = (counter + 1) % len(available_nodes);

        network_settings.CrossPathFunctionality[function_name][functionality.Args["name"]] = new_info
    }

    return network_settings, nil
}

func AssignFunctionalityToInfraNode(function_settings Function, available_nodes []NodeInfo) Function {

    function_settings.Sel4Name = available_nodes[counter].Sel4Name
    function_settings.Mac = available_nodes[counter].Mac
    function_settings.Sel4IP = available_nodes[counter].Sel4IP

    counter = (counter + 1) % len(available_nodes);

    return function_settings
}

