package helpers

import (
    "fmt"
    // "errors"
)

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
            default:
                // Handle or ignore other types if necessary
        }
        result += "\n";
    }
    fmt.Println(result)
}


