package main

import (
	"fmt"
	"os"
	"syscall"

	"github.com/guidorota/pod/net/rtnetlink"
)

func main() {
	infos, err := rtnetlink.GetAllInfo()
	if err != nil {
		fmt.Println("error:", err)
		os.Exit(1)
	}

	for _, info := range infos {
		fmt.Println(info.Ifi.Index)
		a := info.Atts[syscall.IFLA_IFNAME]
		fmt.Println(a.AsString())
	}

	info, err := rtnetlink.GetInfo(1)
	if err != nil {
		fmt.Println("error:", err)
		os.Exit(1)
	}
	fmt.Println(info.Ifi.Index)
	a := info.Atts[syscall.IFLA_IFNAME]
	fmt.Println(a.AsString())

	info, err = rtnetlink.GetInfo(56)
	fmt.Println("error:", err)

	os.Exit(0)
}
