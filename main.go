package main

import (
	"fmt"
	"syscall"

	"github.com/guidorota/pod/net/rtnetlink"
)

func main() {
	infos, err := rtnetlink.GetAllInfo()
	if err != nil {
		fmt.Println("error:", err)
	}

	for _, info := range infos {
		fmt.Println(info.Ifi.Index)
		a := info.Atts[syscall.IFLA_IFNAME]
		fmt.Println(a.AsString())
	}
}
