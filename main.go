package main

import (
	"fmt"
	"os"

	"github.com/guidorota/pod/net"
)

func main() {
	err := net.CreateBridge("tbrdg")
	if err != nil {
		fmt.Println("error")
		os.Exit(1)
	}

	os.Exit(0)
}
