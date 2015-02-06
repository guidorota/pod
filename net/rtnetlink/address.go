package rtnetlink

import (
	"syscall"
	"unsafe"

	"github.com/guidorota/pod/net/netlink"
)

const (
	SizeofIfAddrmsg = syscall.SizeofIfAddrmsg
)

type IfAddrmsg struct {
	Family    uint8
	PrefixLen uint8
	Flags     uint8
	Scope     uint8
	Index     int32
}

type Address struct {
	Ifa  IfAddrmsg
	Atts AttributeList
}

func NewAddress() *Address {
	a := &Address{}
	a.Atts = NewAttributeList()
	return a
}

func (a *Address) Encode() []byte {
	b := make([]byte, SizeofIfAddrmsg)

	*(*IfAddrmsg)(unsafe.Pointer(&b[0])) = a.Ifa
	b = append(b, a.Atts.Encode()...)

	return b
}

func DecodeAddress(b []byte) (*Address, error) {
	a := Address{}

	if len(b) < SizeofIfAddrmsg {
		return nil, netlink.ErrNoData
	}
	a.Ifa = *(*IfAddrmsg)(unsafe.Pointer(&b[0:SizeofIfAddrmsg][0]))

	atts, _, err := DecodeAttributeList(b[SizeofIfAddrmsg:])
	if err != nil {
		return nil, err
	}
	a.Atts = atts

	return &a, nil
}
