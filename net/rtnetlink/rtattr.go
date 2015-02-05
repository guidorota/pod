package rtnetlink

import (
	"strings"
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

type AttList map[int]*Attribute

func NewAttList() AttList {
	return AttList(make(map[int]*Attribute))
}

func (al AttList) Add(att *Attribute) {
	al[int(att.Type)] = att
}

func (al AttList) Get(rt_type int) *Attribute {
	return al[rt_type]
}

func (al AttList) Encode() []byte {
	var b []byte

	for _, a := range al {
		b = append(b, a.Encode()...)
	}

	return b
}

// DecodeAttributeList decodes an rtnetlink rtattr list into an *Attribute map.
func DecodeAttributeList(b []byte) (AttList, []byte, error) {
	am := make(map[int]*Attribute)

	for {
		att, br, err := DecodeAttribute(b)
		if err == netlink.ErrNoData {
			break
		} else if err != nil {
			return nil, b, err
		}
		am[int(att.Type)] = att
		b = br
	}

	return AttList(am), b, nil
}

type Attribute struct {
	Type uint16
	data []byte
}

// DecodeAttribute decodes a single rtnetlink rtattr into an *Attribute.
func DecodeAttribute(b []byte) (*Attribute, []byte, error) {
	if len(b) < SizeofRtAttr {
		return nil, nil, netlink.ErrNoData
	}

	length := *(*uint16)(unsafe.Pointer(&b[0:2][0]))
	if uint16(len(b)) < length ||
		length < SizeofRtAttr {
		return nil, b, netlink.ErrNoData
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

func NewAttr(rt_type uint16, e netlink.Encoder) *Attribute {
	return &Attribute{
		rt_type,
		e.Encode(),
	}
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
	return strings.TrimRight(string(a.data), "\x00")
}
