package rtnetlink

import (
	"errors"
	"syscall"
	"unsafe"

	"github.com/guidorota/pod/net/netlink"
)

const (
	RTA_ALIGNTO  = syscall.RTA_ALIGNTO
	SizeofRtAttr = syscall.SizeofRtAttr
)

// RTA_STRUCT_ALEN represents the aligned length of a struct rtattr
var RTA_STRUCT_ALEN = netlink.Align(SizeofRtAttr, RTA_ALIGNTO)

type Attribute struct {
	Type uint16
	data []byte
}

var ErrNoData = errors.New("cannot parse rtattr, not enough data")

func DecodeAttribute(b []byte) (*Attribute, []byte, error) {
	if len(b) < SizeofRtAttr {
		return nil, nil, ErrNoData
	}

	length := *(*uint16)(unsafe.Pointer(&b[0:2][0]))
	if uint16(len(b)) < length {
		return nil, b, ErrNoData
	}

	a := &Attribute{}
	a.Type = *(*uint16)(unsafe.Pointer(&b[2:4][0]))
	data_len := int(length) - RTA_STRUCT_ALEN
	a.data = make([]byte, data_len)
	copy(a.data, b[RTA_STRUCT_ALEN:length])

	r := netlink.Align(int(length), RTA_ALIGNTO)
	return a, b[r:], nil
}

func (a *Attribute) Encode() []byte {
	l := RTA_STRUCT_ALEN + len(a.data)
	b := make([]byte, netlink.Align(l, RTA_ALIGNTO))

	*(*uint16)(unsafe.Pointer(&b[0:2][0])) = uint16(l)
	*(*uint16)(unsafe.Pointer(&b[2:4][0])) = a.Type
	copy(b[RTA_STRUCT_ALEN:], a.data)

	return b
}

func NewUint8Attr(rt_type uint16, value uint8) *Attribute {
	a := &Attribute{
		rt_type,
		make([]byte, 1),
	}
	*(*uint8)(unsafe.Pointer(&a.data[0])) = value
	return a
}

func (a *Attribute) AsUint8() uint8 {
	return *(*uint8)(unsafe.Pointer(&a.data[0]))
}

func NewInt8Attr(rt_type uint16, value int8) *Attribute {
	a := &Attribute{
		rt_type,
		make([]byte, 1),
	}
	*(*int8)(unsafe.Pointer(&a.data[0])) = value
	return a
}

func (a *Attribute) AsInt8() int8 {
	return *(*int8)(unsafe.Pointer(&a.data[0]))
}

func NewUint16Attr(rt_type, value uint16) *Attribute {
	a := &Attribute{
		rt_type,
		make([]byte, 2),
	}
	*(*uint16)(unsafe.Pointer(&a.data[0:2][0])) = value
	return a
}

func (a *Attribute) AsUint16() uint16 {
	return *(*uint16)(unsafe.Pointer(&a.data[0:2][0]))
}

func NewInt16Attr(rt_type uint16, value int16) *Attribute {
	a := &Attribute{
		rt_type,
		make([]byte, 2),
	}
	*(*int16)(unsafe.Pointer(&a.data[0:2][0])) = value
	return a
}

func (a *Attribute) AsInt16() int16 {
	return *(*int16)(unsafe.Pointer(&a.data[0:2][0]))
}

func NewUint32Attr(rt_type uint16, value uint32) *Attribute {
	a := &Attribute{
		rt_type,
		make([]byte, 4),
	}
	*(*uint32)(unsafe.Pointer(&a.data[0:4][0])) = value
	return a
}

func (a *Attribute) AsUint32() uint32 {
	return *(*uint32)(unsafe.Pointer(&a.data[0:4][0]))
}

func NewInt32Attr(rt_type uint16, value int32) *Attribute {
	a := &Attribute{
		rt_type,
		make([]byte, 4),
	}
	*(*int32)(unsafe.Pointer(&a.data[0:4][0])) = value
	return a
}

func (a *Attribute) AsInt32() int32 {
	return *(*int32)(unsafe.Pointer(&a.data[0:4][0]))
}

func NewStringAttr(rt_type uint16, value string) *Attribute {
	return &Attribute{
		rt_type,
		[]byte(value),
	}
}

func (a *Attribute) AsString() string {
	return string(a.data)
}
