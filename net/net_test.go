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
