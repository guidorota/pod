package main

import (
	"fmt"
	"os"

	"github.com/guidorota/pod/net"
)

func main() {
	err := net.CreateVeth("veth0", "veth1")
	if err != nil {
		fmt.Println("error")
		os.Exit(1)
	}

	os.Exit(0)
}
