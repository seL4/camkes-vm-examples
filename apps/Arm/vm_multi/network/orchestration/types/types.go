package types;

type FunctionalityInfo struct {
    Name      string    
    Sel4Name  string
    Mac       string
    VIDs      []int
    Sel4IP    string
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

type Config struct {
    DataPaths       map[string]DataPath            `yaml:"data_paths"`
    Debug           DebugInfo                      `yaml:"debug"`
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
    Args    map[string]string                      `yaml:"args"`    // Existing function arguments
    VIDS    [2]int                                 // Additional field not mapped from YAML
}

type IngestInfo struct {
    InterfaceName   string                         `yaml:"InterfaceName"`
    PrivateKey      string                         `yaml:"PrivateKey"`
    Address         string                         `yaml:"Address"`
    ListenPort      string                         `yaml:"ListenPort"`
    Peers           map[string]PeerInfo            `yaml:"peers"`
}

type PeerInfo struct {
    PublicKey      string                         `yaml:"PublicKeys"`
    AllowedIPs     []string                       `yaml:"AllowedIPs"`
    Endpoint       string                         `yaml:"Endpoint"`
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

