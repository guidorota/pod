package net

import (
	"testing"
)

func TestIfIndex(t *testing.T) {
	idx, err := ifIndex("lo")
	if err != nil {
		t.Fatal("IfIndex error:", err)
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
