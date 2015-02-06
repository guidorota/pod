package net

import (
	"fmt"
	"net"
	"syscall"

	rt "github.com/guidorota/pod/net/rtnetlink"
)

// Interface represents a network interface.
type Interface int32

// FromName returns an Interface corresponding to an existing network
// interface.
func FromName(name string) (Interface, error) {
	idx, err := ifIndex(name)
	if err != nil {
		return -1, err
	}
	return Interface(idx), nil
}

// NewBridge creates a new bridge interface.
func NewBridge(name string) (Interface, error) {
	if err := checkIfName(name); err != nil {
		return -1, err
	}

	li := rt.NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Flags = syscall.IFF_MULTICAST

	nameAtt := rt.NewStringAttr(syscall.IFLA_IFNAME, name)
	li.Atts.Add(nameAtt)

	kindAtt := rt.NewStringAttr(rt.IFLA_INFO_KIND, "bridge")
	infoAtt := rt.NewAttribute(syscall.IFLA_LINKINFO, kindAtt)
	li.Atts.Add(infoAtt)

	if err := rt.CreateLink(li); err != nil {
		return -1, err
	}

	idx, err := ifIndex(name)
	if err != nil {
		return -1, err
	}
	return Interface(idx), nil
}

// NewVeth creates a new veth pair
func NewVeth(name1, name2 string) (Interface, Interface, error) {
	if err := checkIfName(name1); err != nil {
		return -1, -1, fmt.Errorf("interface 1 name error:", err)
	}
	if err := checkIfName(name2); err != nil {
		return -1, -1, fmt.Errorf("interface 2 name error:", err)
	}

	// VETH_INFO_PEER
	pLi := rt.NewLinkInfo()
	pLi.Ifi.Family = syscall.AF_UNSPEC
	pLi.Ifi.Flags = syscall.IFF_MULTICAST
	pLi.Atts.Add(rt.NewStringAttr(syscall.IFLA_IFNAME, name2))
	vethInfoPeer := rt.NewAttribute(rt.VETH_INFO_PEER, pLi)

	// IFLA_LINKINFO
	iflaLinkInfo := rt.NewAttributeList()
	iflaLinkInfo.Add(rt.NewStringAttr(rt.IFLA_INFO_KIND, "veth"))
	iflaLinkInfo.Add(rt.NewAttribute(rt.IFLA_INFO_DATA, vethInfoPeer))

	li := rt.NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Flags = syscall.IFF_MULTICAST
	li.Atts.Add(rt.NewStringAttr(syscall.IFLA_IFNAME, name1))
	li.Atts.Add(rt.NewAttribute(syscall.IFLA_LINKINFO, iflaLinkInfo))

	if err := rt.CreateLink(li); err != nil {
		return -1, -1, err
	}

	idx1, err := ifIndex(name1)
	if err != nil {
		return -1, -1, err
	}
	idx2, err := ifIndex(name2)
	if err != nil {
		return -1, -1, err
	}

	return Interface(idx1), Interface(idx2), nil
}

func (ifa Interface) Name() (string, error) {
	return ifName(int32(ifa))
}

func (ifa Interface) Addrs() ([]*net.IPNet, error) {
	as, err := rt.GetAddrs()
	if err != nil {
		return nil, err
	}

	var nets []*net.IPNet
	for _, a := range as {
		if a.Ifa.Index != int32(ifa) {
			continue
		}
		ipa := a.Atts.Get(syscall.IFA_ADDRESS)
		if ipa == nil {
			return nil, fmt.Errorf("rtnetlink error, missing IFA_ADDRESS")
		}

		n := &net.IPNet{}
		n.IP = ipa.AsIP()
		n.Mask = net.CIDRMask(int(a.Ifa.PrefixLen), 8*len(n.IP))

		nets = append(nets, n)
	}
	return nets, nil
}

func (ifa Interface) GetAttribute(name int) (*rt.Attribute, error) {
	li, err := rt.GetLinkInfo(int32(ifa))
	if err != nil {
		return nil, err
	}

	return li.Atts.Get(name), nil
}

// DeleteLink removes a network interface from the system
func (ifa Interface) Delete() error {
	return rt.DeleteLink(int32(ifa))
}

// IsUp returns true if the interface is up, false otherwise
func (ifa Interface) IsUp() (bool, error) {
	li, err := rt.GetLinkInfo(int32(ifa))
	if err != nil {
		return false, err
	}

	return (li.Ifi.Flags & syscall.IFF_UP) == 1, nil
}

func changeFlags(idx int32, set, unset uint32) error {
	li, err := rt.GetLinkInfo(idx)
	if err != nil {
		return err
	}

	nli := rt.NewLinkInfo()
	nli.Ifi.Family = li.Ifi.Family
	nli.Ifi.Index = li.Ifi.Index
	nli.Ifi.Flags = (li.Ifi.Flags | set) & ^unset

	return rt.ModifyLink(nli)
}

func (ifa Interface) Down() error {
	return changeFlags(int32(ifa), 0, syscall.IFF_UP)
}

func (ifa Interface) Up() error {
	return changeFlags(int32(ifa), syscall.IFF_UP, 0)
}

func setAttribute(idx int32, att *rt.Attribute) error {
	li, err := rt.GetLinkInfo(idx)
	if err != nil {
		return err
	}

	nli := rt.NewLinkInfo()
	nli.Ifi.Family = li.Ifi.Family
	nli.Ifi.Index = li.Ifi.Index
	nli.Ifi.Flags = li.Ifi.Flags
	nli.Atts.Add(att)

	return rt.ModifyLink(nli)
}

func (ifa Interface) Rename(name string) error {
	att := rt.NewStringAttr(syscall.IFLA_IFNAME, name)
	return setAttribute(int32(ifa), att)
}

func (ifa Interface) SetMaster(master string) error {
	mIdx, err := ifIndex(master)
	if err != nil {
		return fmt.Errorf("cannot find master: %v", err)
	}

	att := rt.NewInt32Attr(syscall.IFLA_MASTER, mIdx)
	return setAttribute(int32(ifa), att)
}

func (ifa Interface) SetNamespace(pid int) error {
	att := rt.NewInt32Attr(syscall.IFLA_NET_NS_PID, int32(pid))
	return setAttribute(int32(ifa), att)
}

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

// ifName returns the interface name given its index
func ifName(idx int32) (string, error) {
	li, err := rt.GetLinkInfo(idx)
	if err != nil {
		return "", err
	}

	a := li.Atts.Get(syscall.IFLA_IFNAME)
	if a == nil {
		return "", fmt.Errorf("rtnetlink error, missing IFLA_IFNAME")
	}
	return a.AsString(), nil
}

// checkIfName returns an error in case the string passed as parameter is not a
// valid interface name.
func checkIfName(name string) error {
	nl := len(name)
	if nl == 0 {
		return fmt.Errorf("empty interface name")
	} else if nl > rt.IF_NAMESIZE {
		return fmt.Errorf("interface name too long")
	}
	return nil
}
