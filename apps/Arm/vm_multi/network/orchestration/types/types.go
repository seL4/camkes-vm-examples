package types;

import (
    "gopkg.in/yaml.v3"
)

type FunctionalityInfo struct {
    Name      string    
    IPs       []string
    Interfaces []string
    /*
    VIDs      []int
    */

}

type NodeInfo struct {
    Name     string
    Mac      string
    Sel4Name string
    VIDs     []int
    IPs      []string
    Sel4IP   string
}

type DebugInfo struct {
    OrderVlans      bool                           `yaml:"order_vlans"`
    Verbose         bool                           `yaml:"verbose"`
}

type IPInfo struct {
    Sel4Name        string
    UserIP          string
    UserName        string    
}

// Name of kinds of service maps to name of particular service which maps to details of that service
type CrossPathFunctionalityMap map[string]map[string]Function

type Config struct {
    DataPaths                             map[string]DataPath            `yaml:"data_paths"`
    Debug                                 DebugInfo                      `yaml:"debug"`
    CrossPathFunctionality                CrossPathFunctionalityMap
}


type DataPath struct {
    Settings        DataPathSettings               `yaml:"settings"`
    Ingest          IngestInfo                     `yaml:"ingest"`
    Functions       map[string]Function            `yaml:"functions"`
    Connections     [][2]int
}

type DataPathSettings struct {
    IP              string                         `yaml:"IP"`
}

type Function struct {
    Args        map[string]string                      `yaml:"-"`    // Existing function arguments
    VIDs        []int                                 // Additional field not mapped from YAML
    Sel4Name    string
    Mac         string
    Sel4IP      string
    Interfaces  []string
    IPs         []string
    Name        string
}

// Custom unmarshal function for Function struct
func (f *Function) UnmarshalYAML(value *yaml.Node) error {
    var tmp map[string]string

    // Unmarshal the YAML node into a map
    if err := value.Decode(&tmp); err != nil {
        return err
    }

    // Store the map in the Args field
    f.Args = tmp
    return nil
}

type IngestInfo struct {
    InterfaceName   string                         `yaml:"InterfaceName"`
    PrivateKey      string                         `yaml:"PrivateKey"`
    Address         string                         `yaml:"Address"`
    ListenPort      string                         `yaml:"ListenPort"`
    Peers           map[string]PeerInfo            `yaml:"peers"`
    VID             int
}

type PeerInfo struct {
    PublicKey      string                         `yaml:"PublicKeys"`
    AllowedIPs     []string                       `yaml:"AllowedIPs"`
    Endpoint       string                         `yaml:"Endpoint"`
}

type YamlParseError struct {
    s string
}

func (e *YamlParseError) Error() string {
    return e.s
}

func NewYamlParseError(message string) *YamlParseError {
    return &YamlParseError{s: message}
}


type CrossDataPathFunctionalityError struct {
    s string
}

func (e *CrossDataPathFunctionalityError) Error() string {
    return e.s
}

func NewCrossDataPathFunctionalityError(message string) *YamlParseError {
    return &YamlParseError{s: message}
}


// type Functions struct {
//     Name           String             ``
//     Args           map[String]String  ``
// }



/*
type Link struct {
    From string `yaml:"from"`
    To   string `yaml:"to"`
}

type Cluster struct {
    Settings      Settings      `yaml:"settings"`
    Nodes         []string      `yaml:"nodes"`
    Connections   []Connection  `yaml:"connections"`
    Functionality []string      `yaml:"functionality"`
}

type Settings struct {
    LinkTypeDefault string `yaml:"link_type_default"`
}

type Connection struct {
    From    string   `yaml:"from"`
    Through []string `yaml:"through"`
    To      string   `yaml:"to"`
    Type    string   `yaml:"type"`
}

type Topology struct {
    Settings Settings `yaml:"settings"`
    Links    []Link   `yaml:"links"`
}
*/

