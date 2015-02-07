package net

import (
	"bytes"
	"net"
	"syscall"
	"testing"
)

const (
	bridgeName = "test_bridge"
	veth0Name  = "test_veth0"
	veth1Name  = "test_veth1"
)

func TestIfIndex(t *testing.T) {
	idx, err := ifIndex("lo")
	if err != nil {
		t.Fatal("ifIndex error:", err)
	}
	if idx < 1 {
		t.Error("wrong index")
	}

	idx, err = ifIndex("asf")
	if err == nil {
		t.Error("interface should not have been found")
	}
	if idx > 0 {
		t.Error("wrong interface index found")
	}
}

func TestIfName(t *testing.T) {
	name, err := ifName(1)
	if err != nil {
		t.Fatal("ifName error:", err)
	}
	if name != "lo" {
		t.Error("wrong name, expected lo but got", name)
	}

	name, err = ifName(0)
	if err == nil {
		t.Fatal("error expected when probing interface nr. 0")
	}
}

func TestCheckIfName(t *testing.T) {
	okName := "ok"
	longName := "0123456789ABCDEF_"
	emptyName := ""

	if err := checkIfName(okName); err != nil {
		t.Error("error on correct name")
	}

	if err := checkIfName(longName); err == nil {
		t.Error("no error on long name")
	}

	if err := checkIfName(emptyName); err == nil {
		t.Error("no error on empty name")
	}
}

func TestFromName(t *testing.T) {
	_, err := FromName("lo")
	if err != nil {
		t.Fatal("missing lo interface")
	}
}

func TestNewBridge(t *testing.T) {
	br, err := NewBridge(bridgeName)
	if err != nil {
		t.Fatal("error creating bridge")
	}
	defer func() {
		if err := br.Delete(); err != nil {
			t.Error("error deleting bridge")
		}
	}()

	n, err := br.Name()
	if err != nil {
		t.Error("error retrieving bridge name")
	}
	if n != bridgeName {
		t.Error("wrong bridge name")
	}
}

func TestNewVeth(t *testing.T) {
	if0, if1, err := NewVeth(veth0Name, veth1Name)
	if err != nil {
		t.Fatal("error creating veth pair")
	}
	defer func() {
		if err := if0.Delete(); err != nil {
			t.Error("error deleting veth pair")
		}
	}()

	n0, err := if0.Name()
	if err != nil {
		t.Error("error retrieving veth0 name")
	}
	if n0 != veth0Name {
		t.Error("wrong veth0 name")
	}

	n1, err := if1.Name()
	if err != nil {
		t.Error("error retrieving veth1 name")
	}
	if n1 != veth1Name {
		t.Error("wrong veth1 name")
	}
}

func TestUpDown(t *testing.T) {
	br, err := NewBridge(bridgeName)
	if err != nil {
		t.Fatal("error creating bridge")
	}
	defer func() {
		if err := br.Delete(); err != nil {
			t.Error("error deleting bridge")
		}
	}()

	up, err := br.IsUp()
	if err != nil {
		t.Error("error probing interface status")
	}
	if up {
		t.Error("interface should have not been created up")
	}

	if err := br.Up(); err != nil {
		t.Error("error bringing the interface up:", err)
	}
	up, err = br.IsUp()
	if err != nil {
		t.Error("error probing interface status")
	}
	if !up {
		t.Error("interface was not brought up properly")
	}

	if err := br.Down(); err != nil {
		t.Error("error bringing the interface up:", err)
	}
	up, err = br.IsUp()
	if err != nil {
		t.Error("error probing interface status")
	}
	if up {
		t.Error("interface was not brought down properly")
	}
}

func TestRename(t *testing.T) {
	br, err := NewBridge(bridgeName)
	if err != nil {
		t.Fatal("error creating bridge")
	}
	defer func() {
		if err := br.Delete(); err != nil {
			t.Error("error deleting bridge")
		}
	}()

	n, err := br.Name()
	if err != nil {
		t.Error("error retrieving interface name")
	}
	if n != bridgeName {
		t.Error("wrong bridge name")
	}

	newName := bridgeName + "new"
	if err := br.Rename(newName); err != nil {
		t.Error("error renaming bridge")
	}

	n, err = br.Name()
	if err != nil {
		t.Error("error retrieving interface name")
	}
	if n != newName {
		t.Error("bridge was not renamed properly")
	}
}

func TestSetMaster(t *testing.T) {
	br, err := NewBridge(bridgeName)
	if err != nil {
		t.Fatal("error creating bridge")
	}
	defer func() {
		if err := br.Delete(); err != nil {
			t.Error("error deleting bridge")
		}
	}()

	v0, _, err := NewVeth(veth0Name, veth1Name)
	if err != nil {
		t.Error("error creating veth pair")
		return
	}
	defer func() {
		if err := v0.Delete(); err != nil {
			t.Error("error deleting veth pair")
		}
	}()

	if err := v0.SetMaster(bridgeName); err != nil {
		t.Error("error setting master interface")
	}

	att, err := v0.GetAttribute(syscall.IFLA_MASTER)
	if err != nil {
		t.Error("error retrieving attribute IFLA_MASTER")
	}
	if att == nil {
		t.Error("attribute IFLA_MASTER does not exist")
	}
	if att.AsInt32() != int32(br) {
		t.Error("wrong master")
	}
}

func TestAddressEqual(t *testing.T) {
	a1, err1 := ParseCIDR("127.0.0.1/24")
	a2, err2 := ParseCIDR("127.0.0.1/16")
	a3, err3 := ParseCIDR("87.3.2.4/24")
	if err1 != nil || err2 != nil || err3 != nil {
		t.Fatal("error while parsing address in cidr format")
	}

	if !a1.Equal(a1) || !a2.Equal(a2) || !a3.Equal(a3) {
		t.Error("equal is not reflexive")
	}

	if a1.Equal(a2) || a1.Equal(a3) || a2.Equal(a3) {
		t.Error("addresses are mistakenly recognized as equal")
	}
}

func TestParseCIDR(t *testing.T) {
	a, err := ParseCIDR("173.231.32.45/12")
	if err != nil {
		t.Fatal("error while parsing address in cidr format")
	}

	ip := net.IPv4(173, 231, 32, 45)
	if !a.IP.Equal(ip) {
		t.Error("ip address not parsed correctly")
	}
	mask := net.CIDRMask(12, 8*net.IPv4len)
	if bytes.Compare(a.Mask, mask) != 0 {
		t.Error("mask not parsed correctly")
	}
}

func TestGetAddr(t *testing.T) {
	lo, err := FromName("lo")
	if err != nil {
		t.Fatal("error accessing loopback interface")
	}

	as, err := lo.Addrs()
	if err != nil {
		t.Fatal("error reading addresses")
	}
	if len(as) == 0 {
		t.Fatal("no addresses found")
	}
}

func TestSetAddr(t *testing.T) {
	br, err := NewBridge(bridgeName)
	if err != nil {
		t.Fatal("error creating bridge")
	}
	defer func() {
		if err := br.Delete(); err != nil {
			t.Error("error deleting bridge")
		}
	}()

	addr, err := ParseCIDR("12.13.14.15/24")
	if err != nil {
		t.Error("error parsing address in cidr format")
		return
	}
	if err := br.SetAddr(addr); err != nil {
		t.Error("error setting address")
		return
	}

	as, err := br.Addrs()
	if err != nil {
		t.Error("error fetching bridge addresses")
	}

	for _, a := range as {
		if a.Equal(addr) {
			return
		}
	}
	t.Error("address was not set")
}
