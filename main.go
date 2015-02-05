package main

import (
	"fmt"
	"os"
	"syscall"

	"github.com/guidorota/pod/net/rtnetlink"
)

func main() {
	kind := rtnetlink.NewStringAttr(rtnetlink.IFLA_INFO_KIND, "bridge")
	linfo := rtnetlink.NewAttr(syscall.IFLA_LINKINFO, kind)
	li := rtnetlink.NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Flags = syscall.IFF_MULTICAST
	li.Atts.Add(rtnetlink.NewStringAttr(syscall.IFLA_IFNAME, "tbrdg"))
	li.Atts.Add(linfo)

	err := rtnetlink.CreateLink(li)
	if err != nil {
		fmt.Println("error:", err)
		os.Exit(1)
	}

	os.Exit(0)
}
