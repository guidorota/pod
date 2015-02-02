package rtnetlink

import (
	"syscall"
	"unsafe"

	"github.com/guidorota/pod/net/netlink"
)

type Ifinfomsg struct {
	Family uint16
	Type   uint16
	Index  int32
	Flags  uint32
	Change uint32
}

type LinkInfo struct {
	Ifi  Ifinfomsg
	Atts []Attribute
}

func (l *LinkInfo) Encode() []byte {
	b := make([]byte, 16)
	l.Ifi.Change = 0xFFFFFFFF

	*(*Ifinfomsg)(unsafe.Pointer(&b[0])) = l.Ifi

	for _, a := range l.Atts {
		b = append(b, a.Encode()...)
	}

	return b
}

func GetAllInfo() ([]LinkInfo, error) {
	req := &netlink.Message{}
	req.Type = syscall.RTM_GETLINK
	req.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_DUMP

	_, err := request(req)
	if err != nil {
		return nil, err
	}

	return nil, nil
}
