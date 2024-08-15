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
    OrderVlans bool    `yaml:"order_vlans"`
    Verbose    bool    `yaml:"verbose"`
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

type Link struct {
    From string `yaml:"from"`
    To   string `yaml:"to"`
}

type Config struct {
    Clusters        map[string]Cluster `yaml:"clusters"`
    ClusterTopology Topology           `yaml:"topology"`
    Debug           DebugInfo          `yaml:"debug"`
}

type IPInfo struct {
    Sel4Name     string
    UserIP       string
    UserName     string    
}

