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

type AddrInfo struct {
	Ifa  IfAddrmsg
	Atts AttributeList
}

func NewAddrInfo() *AddrInfo {
	a := &AddrInfo{}
	a.Atts = NewAttributeList()
	return a
}

func (a *AddrInfo) Encode() []byte {
	b := make([]byte, SizeofIfAddrmsg)

	*(*IfAddrmsg)(unsafe.Pointer(&b[0])) = a.Ifa
	b = append(b, a.Atts.Encode()...)

	return b
}

func DecodeAddrInfo(b []byte) (*AddrInfo, error) {
	a := AddrInfo{}

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

// GetAddrInfos retrieves the addresses of all available network interfaces.
func GetAddrInfos() ([]*AddrInfo, error) {
	a := &AddrInfo{}

	req := &netlink.Message{}
	req.Type = syscall.RTM_GETADDR
	req.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_DUMP
	req.Append(a)

	msgs, err := request(req)
	if err != nil {
		return nil, err
	}

	var ais []*AddrInfo
	for _, m := range msgs {
		ai, err := DecodeAddrInfo(m.Data())
		if err != nil {
			return nil, err
		}
		ais = append(ais, ai)
	}

	return ais, nil
}

// AddAddr adds an address to a network interface.
func AddAddr(ai *AddrInfo) error {
	req := &netlink.Message{}
	req.Type = syscall.RTM_NEWADDR
	req.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_CREATE |
		syscall.NLM_F_EXCL | syscall.NLM_F_ACK
	req.Append(ai)

	return requestAck(req)
}
