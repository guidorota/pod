package net

import (
	"fmt"
	"syscall"

	"github.com/guidorota/pod/net/rtnetlink"
)

func ifIndex(name string) (int32, error) {
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
			return li.Ifi.Index, nil
		}
	}

	return -1, fmt.Errorf("interface '%v' not found", name)
}

func CreateBridge(name string) error {
	li := rtnetlink.NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Flags = syscall.IFF_MULTICAST

	nameAtt := rtnetlink.NewStringAttr(syscall.IFLA_IFNAME, name)
	li.Atts.Add(nameAtt)

	kindAtt := rtnetlink.NewStringAttr(rtnetlink.IFLA_INFO_KIND, "bridge")
	infoAtt := rtnetlink.NewAttr(syscall.IFLA_LINKINFO, kindAtt)
	li.Atts.Add(infoAtt)

	return rtnetlink.CreateLink(li)
}

func DeleteLink(name string) error {
	idx, err := ifIndex(name)
	if err != nil {
		return err
	}
	return rtnetlink.DeleteLink(idx)
}
