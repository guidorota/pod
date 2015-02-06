package main

import (
	"fmt"
	"os"

	"github.com/guidorota/pod/net"
)

func main() {
	lo, err := net.FromName("lo")
	if err != nil {
		fmt.Println("cannot find lo")
		os.Exit(1)
	}

	as, err := lo.Addrs()
	if err != nil {
		fmt.Println("error fetching addresses")
		os.Exit(1)
	}

	for _, a := range as {
		fmt.Println(a)
	}
	os.Exit(0)
}
