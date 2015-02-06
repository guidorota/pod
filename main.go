package main

import (
	"fmt"
	"net"
	"os"

	cnet "github.com/guidorota/pod/net"
)

func main() {
	br, err := cnet.NewBridge("test_bridge")
	if err != nil {
		fmt.Println("error creating bridge")
		os.Exit(1)
	}

	ip, ipNet, err := net.ParseCIDR("172.17.43.1/26")
	if err != nil {
		fmt.Println("error parsing cidr")
		os.Exit(1)
	}
	err = br.SetAddr(ip, ipNet.Mask)
	if err != nil {
		fmt.Println("error setting address", err)
		os.Exit(1)
	}

	os.Exit(0)
}
