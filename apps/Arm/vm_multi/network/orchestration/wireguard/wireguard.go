package wireguard

import (
    "os"
    "fmt"
    "os/exec"
    "strconv"
    "orchestration/types"
    "orchestration/helpers"
)

type DebugInfo = types.DebugInfo
type IngestInfo = types.IngestInfo


func GetWireGuardInterfaceName(settings IngestInfo, path_num int) string {
    if settings.InterfaceName == "" { settings.InterfaceName = "wg" + strconv.Itoa(path_num); }
    return settings.InterfaceName
}


// path_num is the # data path we are currently building
func BuildWireGuardConfig(settings IngestInfo, path_num int, debug DebugInfo) bool {

    // If there is no wireguard interface name, we are going to set it
    if settings.InterfaceName == "" { 
        settings.InterfaceName = GetWireGuardInterfaceName(settings, path_num) 
    }

    // Create a new file or open an existing file for writing
    file, err := os.Create(fmt.Sprintf("/root/%v.conf", settings.InterfaceName))
    if err != nil {
        helpers.LogE("Error creating wireguard config file:", err)
        return true
    }
    defer file.Close()

    // Write Interface section to the file
    _, err = file.WriteString(fmt.Sprintf("[Interface]\nPrivateKey = %v\nAddress = %v\nListenPort = %v\n\n",
                settings.PrivateKey, settings.Address, settings.ListenPort))
    if err != nil {
        helpers.LogE("Error writing wireguard config file:", err)
        return true
    }

    // Write all the peers into the file
    for _, peer_settings := range settings.Peers {

        // Write intro & pubkey
        _, err = file.WriteString(fmt.Sprintf("[Peer]\nPublicKey = %v\n", peer_settings.PublicKey))
        if err != nil {
            helpers.LogE("Error writing wireguard config file:", err)
            return true
        }

        // Write Allowed IPs section
        allowed_ip_string := "AllowedIPs = "
        for _, ip := range peer_settings.AllowedIPs {
            // ip := ip[:len(ip) - 3] // remove subnet mask
            allowed_ip_string += ip + ", "
        }
        allowed_ip_string = allowed_ip_string[:len(allowed_ip_string) - 2] + "\n"
        _, err = file.WriteString(allowed_ip_string)
        if err != nil {
            helpers.LogE("Error writing wireguard config file:", err)
            return true
        }

        // Write endpoint address section
        _, err = file.WriteString(fmt.Sprintf("Endpoint = %v\n\n", peer_settings.Endpoint))
        if err != nil {
            helpers.LogE("Error writing wireguard config file:", err)
            return true
        }
    }

    if debug.Verbose { helpers.LogE("File written successfully") }

    return false
}

func StartWireGuard(settings IngestInfo, path_num int, debug DebugInfo) bool {

    interface_name := GetWireGuardInterfaceName(settings, path_num)

    if debug.Verbose { helpers.LogE("Setting up wireguard interface ", interface_name) }

    if debug.Verbose { 
        fmt.Println("/root/wireguard-interface.sh", "--interface", interface_name, 
                                "--pk", settings.PrivateKey, "--addr", settings.Address, "--port", settings.ListenPort)
    }
    cmd := exec.Command("/root/wireguard-interface.sh", "--interface", interface_name, 
                            "--pk", settings.PrivateKey, "--addr", settings.Address, "--port", settings.ListenPort)
    output, err := cmd.CombinedOutput()
    if err != nil {
        helpers.LogE("Error creating wireguard interface: ", interface_name, err, output, "Exiting")
        return true
    }


    for name, peer_settings := range settings.Peers {

        // Build allowed ips argument string
        allowed_ips := ""
        for _, ip := range peer_settings.AllowedIPs { 
            // ip = ip[:len(ip) - 3]
            allowed_ips += ip + "," 
        }
        // remove trailing comma
        allowed_ips = allowed_ips[:len(allowed_ips) - 1]
        if debug.Verbose { 
            fmt.Println("Peer name:", name);
            fmt.Println("wg", "set", interface_name, "peer", peer_settings.PublicKey, 
                                    "allowed-ips", allowed_ips, "endpoint", peer_settings.Endpoint)
        }

        cmd := exec.Command("wg", "set", interface_name, "peer", peer_settings.PublicKey, 
                                "allowed-ips", allowed_ips, "endpoint", peer_settings.Endpoint) 

        output, err := cmd.CombinedOutput()
        if err != nil {
            helpers.LogE("Error creating wireguard peer: ", name, err, output, "Exiting")
            return true
        }

    }
    return false
}


