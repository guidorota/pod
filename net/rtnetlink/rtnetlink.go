package rtnetlink

import (
	"syscall"
	"unsafe"

	"github.com/guidorota/pod/net/netlink"
)

func RtaAlign(len int) int {
	return (len + syscall.RTA_ALIGNTO - 1) & ^(syscall.RTA_ALIGNTO - 1)
}

type Attribute struct {
	Type uint16
	Data []byte
}

func (a Attribute) Encode() []byte {
	l := syscall.SizeofRtAttr + len(a.Data)
	b := make([]byte, RtaAlign(l))

	*(*uint16)(unsafe.Pointer(&b[0:2][0])) = uint16(l)
	*(*uint16)(unsafe.Pointer(&b[2:4][0])) = a.Type
	copy(b[4:], a.Data)

	return b
}

func NewUint8Attr(rt_type uint16, value uint8) Attribute {
	a := Attribute{
		rt_type,
		make([]byte, 1),
	}
	*(*uint8)(unsafe.Pointer(&a.Data[0])) = value
	return a
}

func (a *Attribute) AsUint8() uint8 {
	return *(*uint8)(unsafe.Pointer(&a.Data[0]))
}

func NewInt8Attr(rt_type uint16, value int8) Attribute {
	a := Attribute{
		rt_type,
		make([]byte, 1),
	}
	*(*int8)(unsafe.Pointer(&a.Data[0])) = value
	return a
}

func (a *Attribute) AsInt8() int8 {
	return *(*int8)(unsafe.Pointer(&a.Data[0]))
}

func NewUint16Attr(rt_type, value uint16) Attribute {
	a := Attribute{
		rt_type,
		make([]byte, 2),
	}
	*(*uint16)(unsafe.Pointer(&a.Data[0:2][0])) = value
	return a
}

func (a *Attribute) AsUint16() uint16 {
	return *(*uint16)(unsafe.Pointer(&a.Data[0:2][0]))
}

func NewInt16Attr(rt_type uint16, value int16) Attribute {
	a := Attribute{
		rt_type,
		make([]byte, 2),
	}
	*(*int16)(unsafe.Pointer(&a.Data[0:2][0])) = value
	return a
}

func (a *Attribute) AsInt16() int16 {
	return *(*int16)(unsafe.Pointer(&a.Data[0:2][0]))
}

func NewUint32Attr(rt_type uint16, value uint32) Attribute {
	a := Attribute{
		rt_type,
		make([]byte, 4),
	}
	*(*uint32)(unsafe.Pointer(&a.Data[0:4][0])) = value
	return a
}

func (a *Attribute) AsUint32() uint32 {
	return *(*uint32)(unsafe.Pointer(&a.Data[0:4][0]))
}

func NewInt32Attr(rt_type uint16, value int32) Attribute {
	a := Attribute{
		rt_type,
		make([]byte, 4),
	}
	*(*int32)(unsafe.Pointer(&a.Data[0:4][0])) = value
	return a
}

func (a *Attribute) AsInt32() int32 {
	return *(*int32)(unsafe.Pointer(&a.Data[0:4][0]))
}

func NewStringAttr(rt_type uint16, value string) Attribute {
	return Attribute{
		rt_type,
		[]byte(value),
	}
}

func (a *Attribute) AsString() string {
	return string(a.Data)
}

// kernel is the netlink address for the Linux kernel
var kernel = &syscall.SockaddrNetlink{
	Family: syscall.AF_NETLINK,
	Pid:    0,
}

// request is a utility method that opens a NETLINK_ROUTE connection, sends a
// single request message and waits for the related response.
func request(msg *netlink.Message) ([]*netlink.Message, error) {
	c, err := netlink.Connect(syscall.NETLINK_ROUTE)
	if err != nil {
		return nil, err
	}
	defer c.Close()

	if err := c.Send(kernel, msg); err != nil {
		return nil, err
	}

	msgs, err := c.Recv()
	if err != nil {
		return nil, err
	}

	return msgs, nil
}
