package rtnetlink

import (
	"fmt"
	"syscall"
	"unsafe"

	"github.com/guidorota/pod/net/netlink"
)

const (
	IF_NAMESIZE = 16
)

const (
	IFLA_INFO_UNSPEC = iota
	IFLA_INFO_KIND   = iota
	IFLA_INFO_DATA   = iota
	IFLA_INFO_XSTATS = iota
)

const (
	VETH_INFO_UNSPEC = iota
	VETH_INFO_PEER   = iota
)

const (
	SizeofIfInfomsg = syscall.SizeofIfInfomsg
)

type IfInfomsg struct {
	Family uint16
	Type   uint16
	Index  int32
	Flags  uint32
	Change uint32
}

type LinkInfo struct {
	Ifi  IfInfomsg
	Atts AttributeList
}

func NewLinkInfo() *LinkInfo {
	li := &LinkInfo{}
	li.Ifi.Change = 0xFFFFFFFF
	li.Atts = NewAttributeList()
	return li
}

func (li *LinkInfo) Encode() []byte {
	b := make([]byte, SizeofIfInfomsg)
	li.Ifi.Change = 0xFFFFFFFF

	*(*IfInfomsg)(unsafe.Pointer(&b[0])) = li.Ifi
	b = append(b, li.Atts.Encode()...)

	return b
}

func DecodeLinkInfo(b []byte) (*LinkInfo, error) {
	li := LinkInfo{}

	if len(b) < SizeofIfInfomsg {
		return nil, netlink.ErrNoData
	}
	li.Ifi = *(*IfInfomsg)(unsafe.Pointer(&b[0:SizeofIfInfomsg][0]))

	atts, _, err := DecodeAttributeList(b[SizeofIfInfomsg:])
	if err != nil {
		return nil, err
	}
	li.Atts = atts

	return &li, nil
}

// GetLinkInfo returns information regarding a single network interface.
func GetLinkInfo(idx int32) (*LinkInfo, error) {
	li := NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Index = idx

	req := &netlink.Message{}
	req.Type = syscall.RTM_GETLINK
	req.Flags = syscall.NLM_F_REQUEST
	req.Append(li)

	msgs, err := request(req)
	if err != nil {
		return nil, err
	}
	if len(msgs) != 1 {
		return nil, fmt.Errorf("unexpected number of response messages")
	}

	m := msgs[0]
	if m.IsError() {
		return nil, m.Error()
	}
	if m.IsAck() {
		return nil, fmt.Errorf("unexpected ack reply")
	}

	return DecodeLinkInfo(m.Data())
}

// GetAllLinkInfo returns information regarding all network interfaces.
func GetAllLinkInfo() ([]*LinkInfo, error) {
	li := NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC

	req := &netlink.Message{}
	req.Type = syscall.RTM_GETLINK
	req.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_DUMP
	req.Append(li)

	msgs, err := request(req)
	if err != nil {
		return nil, err
	}

	var lis []*LinkInfo
	for _, m := range msgs {
		i, err := DecodeLinkInfo(m.Data())
		if err != nil {
			return nil, err
		}
		lis = append(lis, i)
	}

	return lis, nil
}

func DeleteLink(idx int32) error {
	li := NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Index = idx

	req := &netlink.Message{}
	req.Type = syscall.RTM_DELLINK
	req.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_ACK
	req.Append(li)

	return requestAck(req)
}

// CreateLink instructs the kernel to create a new network interface.
// This function does not allow the end user to change the properties of a
// existing interface.
func CreateLink(li *LinkInfo) error {
	req := &netlink.Message{}
	req.Type = syscall.RTM_NEWLINK
	req.Flags = syscall.NLM_F_CREATE | syscall.NLM_F_EXCL |
		syscall.NLM_F_REQUEST | syscall.NLM_F_ACK
	req.Append(li)

	return requestAck(req)
}

// ModifyLink changes the properties of an existing network interface.
func ModifyLink(li *LinkInfo) error {
	req := &netlink.Message{}
	req.Type = syscall.RTM_NEWLINK
	req.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_ACK
	req.Append(li)

	return requestAck(req)
}
