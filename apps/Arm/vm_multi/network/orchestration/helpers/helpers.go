package helpers

import (
    "fmt"
    "reflect"
    "os"
    "io"
    "strconv"
    "gopkg.in/yaml.v3"
    "orchestration/types"
    // "errors"
)

type Config  = types.Config
type NodeInfo = types.NodeInfo
type IPInfo = types.IPInfo
type FunctionalityInfo = types.FunctionalityInfo

func LogE(args ...interface{}) {
    result := "";
    for _, arg := range args {
        switch v := arg.(type) {
            case string:
                result += v
            case error:
                result += v.Error()
            case []string:
                for _, el := range v {
                    result += el + ", "
                }
            case []uint8:
                for _, el := range v {
                    result += strconv.Itoa(int(el)) + ", "
                }
            case []int:
                for _, el := range v {
                    result += strconv.Itoa(el) + ", "
                }
            case [][2]int:
                result += "[ "
                for _, el := range v {
                    result += "[" + strconv.Itoa(el[0]) + ", " + strconv.Itoa(el[1]) + "], "
                }
                result =  result[:len(result) - 2]
                result += " ]"
            case []FunctionalityInfo:
                for _, el := range v {
                    result += fmt.Sprintf("Name: %v\n", el.Name)
                    // result += fmt.Sprintf("  Mac: %v\n", el.Mac)
                    // result += fmt.Sprintf("  Sel4Name: %v\n", el.Sel4Name)
                    // result += fmt.Sprintf("  VIDS:\n    ")
                    // for _, vid := range el.VIDs { result += fmt.Sprintf("%v,", vid) }
                    //result += "\n"
                    // for _, ip := range el.Sel4IP { result +=  }
                    // result += fmt.Sprintf("  IPs: %v\n\n", el.Sel4IP)
                }
             case []NodeInfo:
                for _, el := range v {
                    result += fmt.Sprintf("Name: %v\n", el.Name)
                    result += fmt.Sprintf("  Mac: %v\n", el.Mac)
                    result += fmt.Sprintf("  Sel4Name: %v\n", el.Sel4Name)
                    result += fmt.Sprintf("  VIDS:\n    ")
                    for _, vid := range el.VIDs { result += fmt.Sprintf("%v,", vid) }
                    result += "\n"
                    result += fmt.Sprintf("  IPs:\n")
                    for _, ip := range el.IPs { result += fmt.Sprintf("    %v\n", ip) }
                    result += fmt.Sprintf("  Sel4IP: %v\n\n", el.Sel4IP)
                }
             case [][2]IPInfo:
                for i, el := range v {
                    result += fmt.Sprintf("IP Info Entry %v:\n", i)
                    result += fmt.Sprintf("  Element 1:\n")
                    result += fmt.Sprintf("    Sel4 Name: %v\n", el[0].Sel4Name)
                    result += fmt.Sprintf("    User IP: %v\n", el[0].UserIP)
                    result += fmt.Sprintf("    User Name: %v\n\n", el[0].UserName)
                    result += fmt.Sprintf("  Element 2:\n")
                    result += fmt.Sprintf("    Sel4 IP: %v\n", el[1].Sel4Name)
                    result += fmt.Sprintf("    User IP: %v\n", el[1].UserIP)
                    result += fmt.Sprintf("    User Name: %v\n\n", el[1].UserName)
                }
            case []interface{}:
                for _, el := range v {
                    result += fmt.Sprintf("%v, ", el)
                }
            default:
                // Handle or ignore other types if necessary
                fmt.Println("Passed in type that can't be printed by LogE: ", reflect.TypeOf(arg))
        }
        result += "\n";
    }
    fmt.Println(result)
}

func InArray[T comparable](el T, el_array []T) bool {
    for _, array_el := range(el_array) {
        if array_el == el { return true }
    }
    return false
}

func GetKeys[T comparable, V any](map_in map[T]V) []T {
    keys := make([]T, len(map_in))

    i := 0
    for k := range map_in {
        keys[i] = k
        i += 1
    }
    return keys
}

func LoadSettings(filename string) (Config, error) {
    file, err := os.Open(filename)
    defer file.Close()
    if err != nil {
        LogE("Error opening config file:", err)
        return Config{}, err
    }

    var parsed_yaml Config

    decoder := yaml.NewDecoder(file)
    decoder.Decode(&parsed_yaml)
    
    if err = decoder.Decode(parsed_yaml); err != nil && err != io.EOF {
        LogE("error decoding YAML: ", err)
        return Config{}, err
    }


    return parsed_yaml, nil
}

func GenerateIp(sel4Name string, vid int) string {
    upper8 := int((vid >> 8) & 0xFF)
    lower8 := int(vid & 0xFF)
    ip := "10." + fmt.Sprintf("%d", upper8) + "." + fmt.Sprintf("%d", lower8) + "." + sel4Name[2:] + "/24"
    return ip
}

