package main

import (
	"fmt"
	"os"

	net "github.com/guidorota/pod/net"
)

func main() {
	br, err := net.NewBridge("test_bridge")
	if err != nil {
		fmt.Println("error creating bridge")
		os.Exit(1)
	}

	addr, err := net.ParseCIDR("172.17.43.1/26")
	if err != nil {
		fmt.Println("error parsing cidr")
		os.Exit(1)
	}
	err = br.SetAddr(addr)
	if err != nil {
		fmt.Println("error setting address", err)
		os.Exit(1)
	}

	os.Exit(0)
}
