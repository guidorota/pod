package net

import (
	"fmt"
	"syscall"

	"github.com/guidorota/pod/net/rtnetlink"
)

func IfIndex(name string) (int, error) {
	lis, err := rtnetlink.GetAllLinkInfo()
	if err != nil {
		return -1, err
	}

	for _, li := range lis {
		a := li.Atts.Get(syscall.IFLA_IFNAME)
		if a == nil {
			return -1, fmt.Errorf("rtnetlink error, missing IFLA_IFNAME")
		}
		if name == a.AsString() {
			return int(li.Ifi.Index), nil
		}
	}

	return -1, fmt.Errorf("interface '%v' not found", name)
}
