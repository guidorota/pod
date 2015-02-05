package net

import (
	"fmt"
	"syscall"

	rt "github.com/guidorota/pod/net/rtnetlink"
)

// ifIndex looks up the index of the network interface whose name is passed as
// parameter.
func ifIndex(name string) (int32, error) {
	lis, err := rt.GetAllLinkInfo()
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

func checkIfName(name string) error {
	nl := len(name)
	if nl == 0 {
		return fmt.Errorf("empty interface name")
	} else if nl > rt.IF_NAMESIZE {
		return fmt.Errorf("interface name too long")
	}
	return nil
}

// CreateBridge creates a new bridge interface.
func CreateBridge(name string) error {
	if err := checkIfName(name); err != nil {
		return err
	}

	li := rt.NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Flags = syscall.IFF_MULTICAST

	nameAtt := rt.NewStringAttr(syscall.IFLA_IFNAME, name)
	li.Atts.Add(nameAtt)

	kindAtt := rt.NewStringAttr(rt.IFLA_INFO_KIND, "bridge")
	infoAtt := rt.NewAttr(syscall.IFLA_LINKINFO, kindAtt)
	li.Atts.Add(infoAtt)

	return rt.CreateLink(li)
}

func CreateVeth(name, peer string) error {
	if err := checkIfName(name); err != nil {
		return fmt.Errorf("name error:", err)
	}
	if err := checkIfName(peer); err != nil {
		return fmt.Errorf("peer name error:", err)
	}

	// VETH_INFO_PEER
	pLi := rt.NewLinkInfo()
	pLi.Ifi.Family = syscall.AF_UNSPEC
	pLi.Ifi.Flags = syscall.IFF_MULTICAST
	pLi.Atts.Add(rt.NewStringAttr(syscall.IFLA_IFNAME, peer))
	vethInfoPeer := rt.NewAttr(rt.VETH_INFO_PEER, pLi)

	// IFLA_LINKINFO
	iflaLinkInfo := rt.NewAttributeList()
	iflaLinkInfo.Add(rt.NewStringAttr(rt.IFLA_INFO_KIND, "veth"))
	iflaLinkInfo.Add(rt.NewAttr(rt.IFLA_INFO_DATA, vethInfoPeer))

	li := rt.NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Flags = syscall.IFF_MULTICAST
	li.Atts.Add(rt.NewStringAttr(syscall.IFLA_IFNAME, name))
	li.Atts.Add(rt.NewAttr(syscall.IFLA_LINKINFO, iflaLinkInfo))

	return rt.CreateLink(li)
}

// DeleteLink removes a network interface from the system
func DeleteLink(name string) error {
	idx, err := ifIndex(name)
	if err != nil {
		return err
	}
	return rt.DeleteLink(idx)
}

// IsUp returns true if the interface is up, false otherwise
func IsUp(name string) (bool, error) {
	idx, err := ifIndex(name)
	if err != nil {
		return false, err
	}

	li, err := rt.GetLinkInfo(idx)
	if err != nil {
		return false, err
	}

	return (li.Ifi.Flags & syscall.IFF_UP) == 1, nil
}
