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

func (l *LinkInfo) Encode() []byte {
	b := make([]byte, 16)
	l.Ifi.Change = 0xFFFFFFFF

	*(*IfInfomsg)(unsafe.Pointer(&b[0])) = l.Ifi

	for _, a := range l.Atts {
		b = append(b, a.Encode()...)
	}

	return b
}

func decodeLinkInfo(m *netlink.Message) (*LinkInfo, error) {
	info := LinkInfo{
		Atts: make(map[int]*Attribute),
	}
	b := m.Data()

	if len(b) < syscall.SizeofIfInfomsg {
		return nil, netlink.ErrNoData
	}
	info.Ifi = *(*IfInfomsg)(unsafe.Pointer(&b[0:SizeofIfInfomsg][0]))
	b = b[SizeofIfInfomsg:]

	for {
		att, br, err := DecodeAttribute(b)
		if err == netlink.ErrNoData {
			break
		} else if err != nil {
			return nil, err
		}
		info.Atts[int(att.Type)] = att
		b = br
	}

	return &info, nil
}

func GetAllInfo() ([]*LinkInfo, error) {
	linfo := &LinkInfo{}
	linfo.Ifi.Family = syscall.AF_UNSPEC

	req := &netlink.Message{}
	req.Type = syscall.RTM_GETLINK
	req.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_DUMP
	req.Append(linfo)

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
		i, err := decodeLinkInfo(m)
		if err != nil {
			return nil, err
		}
		infos = append(infos, i)
	}

	return infos, nil
}
