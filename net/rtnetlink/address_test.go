package rtnetlink

import (
	"testing"
	"unsafe"
)

func TestAddressEncode(t *testing.T) {
	a := &Address{}
	a.Ifa.Family = 12
	a.Ifa.PrefixLen = 8
	a.Ifa.Flags = 43
	a.Ifa.Scope = 84
	a.Ifa.Index = 21

	b := a.Encode()
	if len(b) < SizeofIfAddrmsg {
		t.Fatal("address partially encoded")
	}

	if *(*uint8)(unsafe.Pointer(&b[0])) != a.Ifa.Family {
		t.Error("wrong family")
	}
	if *(*uint8)(unsafe.Pointer(&b[1])) != a.Ifa.PrefixLen {
		t.Error("wrong prefix length")
	}
	if *(*uint8)(unsafe.Pointer(&b[2])) != a.Ifa.Flags {
		t.Error("wrong flags")
	}
	if *(*uint8)(unsafe.Pointer(&b[3])) != a.Ifa.Scope {
		t.Error("wrong scope")
	}
	if *(*int32)(unsafe.Pointer(&b[4:8][0])) != a.Ifa.Index {
		t.Error("wrong index")
	}
}

func TestAddressDecode(t *testing.T) {
	a := &Address{}
	a.Ifa.Family = 12
	a.Ifa.PrefixLen = 8
	a.Ifa.Flags = 43
	a.Ifa.Scope = 84
	a.Ifa.Index = 21

	dec, err := DecodeAddress(a.Encode())
	if err != nil {
		t.Fatal("error decoding address")
	}

	if dec.Ifa.Family != a.Ifa.Family {
		t.Error("wrong family")
	}
	if dec.Ifa.PrefixLen != a.Ifa.PrefixLen {
		t.Error("wrong prefix length")
	}
	if dec.Ifa.Flags != a.Ifa.Flags {
		t.Error("wrong flags")
	}
	if dec.Ifa.Scope != a.Ifa.Scope {
		t.Error("wrong scope")
	}
	if dec.Ifa.Index != a.Ifa.Index {
		t.Error("wrong index")
	}
}
