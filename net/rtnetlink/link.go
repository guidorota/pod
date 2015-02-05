package rtnetlink

import (
	"fmt"
	"syscall"
	"unsafe"

	"github.com/guidorota/pod/net/netlink"
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
	Atts map[int]*Attribute
}

func NewLinkInfo() *LinkInfo {
	li := &LinkInfo{}

	li.Ifi.Change = 0xFFFFFFFF
	li.Atts = make(map[int]*Attribute)

	return li
}

func (l *LinkInfo) Encode() []byte {
	b := make([]byte, 16)
	l.Ifi.Change = 0xFFFFFFFF

	*(*IfInfomsg)(unsafe.Pointer(&b[0])) = l.Ifi

	for _, a := range l.Atts {
		b = append(b, a.Encode()...)
	}

	return b
}

func DecodeLinkInfo(b []byte) (*LinkInfo, error) {
	li := LinkInfo{}

	if len(b) < syscall.SizeofIfInfomsg {
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

	var infos []*LinkInfo
	for _, m := range msgs {
		if m.IsError() {
			return nil, m.Error()
		}
		if m.IsAck() {
			return nil, fmt.Errorf("unexpected ack reply")
		}
		i, err := DecodeLinkInfo(m.Data())
		if err != nil {
			return nil, err
		}
		infos = append(infos, i)
	}

	return infos, nil
}

func DeleteLink(idx int32) error {
	li := NewLinkInfo()
	li.Ifi.Family = syscall.AF_UNSPEC
	li.Ifi.Index = idx

	req := &netlink.Message{}
	req.Type = syscall.RTM_DELLINK
	req.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_ACK
	req.Append(li)

	msgs, err := request(req)
	if err != nil {
		return err
	}
	if len(msgs) != 1 {
		return fmt.Errorf("unexpected number of response messages")
	}

	m := msgs[0]
	if m.IsError() {
		return m.Error()
	}
	if !m.IsAck() {
		return fmt.Errorf("no ack received")
	}

	return nil
}
