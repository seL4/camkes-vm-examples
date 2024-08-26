package main

import (
    "os"
    "bufio"
    "os/exec"
    "fmt"
    "net"
    "strings"
)

var ext_intr string;

func main() {
        // Figure out what our infra interface is:
	file, err := os.Open("/proc/cmdline")
	if err != nil {
		fmt.Println("Error opening /proc/cmdline:", err)
		return
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	if scanner.Scan() {
		bootArgs := scanner.Text()
		// Parse the boot arguments to find "infra="
		ext_intr = parseBootArgs(bootArgs, "infra")
	}

	if err := scanner.Err(); err != nil {
		fmt.Println("Error reading /proc/cmdline:", err)
                return
	}

	// Listen on a specific port
	listener, err := net.Listen("tcp", ":8080")
	if err != nil {
		fmt.Println("Error listening:", err)
		return
	}
	defer listener.Close()

	fmt.Println("Server is listening on port 8080...")

        // Accept an incoming connection
        conn, err := listener.Accept()
        if err != nil {
                fmt.Println("Error accepting connection:", err)
        }

        fmt.Println("Accepted connection")

        // Handle the connection in a new goroutine
        handleConnection(conn)
}

func handleConnection(conn net.Conn) {
	defer conn.Close()

	// Read data from the connection
	message, _ := bufio.NewReader(conn).ReadString('\n')
        message = message[:len(message) - 1] // Remove trailing space so the prog doesn't die
	fmt.Println("Received:", message)

        // addresses=ip1:vlan,ip2:vlan
        addresses := strings.Split(strings.Split(message, "=")[1], ",")

        // Set up all the vlan addresses
        for _, address := range addresses {
            info := strings.Split(address, ":")
            ip := info[0]
            vlan := info[1]

            err := startInterface(ext_intr, ip, vlan)
            if err != nil {
                    fmt.Println("Error:", err)
                    return 
            }
        }

	// Respond to the client
	conn.Write([]byte("Message received.\n"))
}

func startInterface( ext_intr string, ip string, vlan string) error {
        // Figure out what our infra interface is:
        // Add an ip interface for each of these vlans
        command_str := []string{"link", "add", "link", ext_intr, "name",
                    fmt.Sprintf("%s.%s", ext_intr, vlan), "type", "vlan", "id", vlan}
        fmt.Println(command_str)
        err := exec.Command("ip",command_str...).Run()
        if err != nil {
                fmt.Println("Error:", err)
                return err
        }

        command_str = []string{"link", "set", "up", "dev", fmt.Sprintf("%s.%s", ext_intr, vlan)}
        fmt.Println(command_str)
        err = exec.Command("ip", command_str...).Run()
        if err != nil {
                fmt.Println("Error:", err)
                return err
        }

        command_str = []string{"addr", "add", ip, "dev", fmt.Sprintf("%s.%s", ext_intr, vlan)}
        fmt.Println(command_str)
        err = exec.Command("ip", command_str...).Run()
        if err != nil {
                fmt.Println("Error:", err)
                return err
        }
       // Flush the buffer to ensure all data is written to the file
        return nil
}

func parseBootArgs(args, key string) string {
    for _, arg := range strings.Split(args, " ") {
            if strings.HasPrefix(arg, key+"=") {
                    return strings.TrimPrefix(arg, key+"=")
            }
    }
    return ""
}
