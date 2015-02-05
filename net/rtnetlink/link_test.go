package rtnetlink

import (
	"syscall"
	"testing"
	"unsafe"
)

var li = &LinkInfo{}

func init() {
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Type = 53
	li.Ifi.Index = 78
	li.Ifi.Flags = 89
	li.Ifi.Change = 0xFFFFFFFF
}

func TestLinkInfoEncode(t *testing.T) {
	b := li.Encode()
	if len(b) < SizeofIfInfomsg {
		t.Error("linkinfo not decoded")
	}

	if *(*uint16)(unsafe.Pointer(&b[0:2][0])) != li.Ifi.Family {
		t.Error("wrong family")
	}
	if *(*uint16)(unsafe.Pointer(&b[2:4][0])) != li.Ifi.Type {
		t.Error("wrong type")
	}
	if *(*int32)(unsafe.Pointer(&b[4:8][0])) != li.Ifi.Index {
		t.Error("wrong index")
	}
	if *(*uint32)(unsafe.Pointer(&b[8:12][0])) != li.Ifi.Flags {
		t.Error("wrong flags")
	}
	if *(*uint32)(unsafe.Pointer(&b[12:16][0])) != li.Ifi.Change {
		t.Error("wrong change")
	}
}

func TestLinkInfoDecode(t *testing.T) {
	new_li, err := DecodeLinkInfo(li.Encode())
	if err != nil {
		t.Fatal("error decoding", err)
	}

	if new_li.Ifi.Family != li.Ifi.Family {
		t.Error("wrong family")
	}
	if new_li.Ifi.Type != li.Ifi.Type {
		t.Error("wrong type")
	}
	if new_li.Ifi.Index != li.Ifi.Index {
		t.Error("wrong index")
	}
	if new_li.Ifi.Flags != li.Ifi.Flags {
		t.Error("wrong flags")
	}
	if new_li.Ifi.Change != li.Ifi.Change {
		t.Error("wrong change")
	}
}

func TestGetAllInfo(t *testing.T) {
	lis, err := GetAllInfo()
	if err != nil {
		t.Fatal("error retrieving interface information", err)
	}

	if len(lis) == 0 {
		t.Fatal("no information retrieved")
	}
}

func TestGetInfo(t *testing.T) {
	li, err := GetInfo(1)
	if err != nil {
		t.Fatal("error retrieving interface information", err)
	}

	if li.Ifi.Index != 1 {
		t.Error("wrong index")
	}

	a, ok := li.Atts[syscall.IFLA_IFNAME]
	if !ok {
		t.Fatal("missing interface name")
	}
	if a.AsString() != "lo" {
		t.Error("wrong interface name")
	}
}
