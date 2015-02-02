package netlink

import (
	"syscall"
	"unsafe"
)

type Encoder interface {
	Encode(alignTo int) []byte
}

func Align(len, alignTo int) int {
	return (len + alignTo - 1) & ^(alignTo - 1)
}

type Message struct {
	Type  uint16
	Flags uint16
	Seq   uint32
	Pid   uint32
	data  []byte
}

func (m *Message) Append(e Encoder) {
	m.data = append(m.data, e.Encode(syscall.NLMSG_ALIGNTO)...)
}

// GetErrorCode returns the error code associated with the netlink message.
// This function returns 0 if the message does not contain any error.
func (m *Message) GetErrorCode() int {
	if m.Type != syscall.NLMSG_ERROR {
		return 0
	}
	ecode := *(*int32)(unsafe.Pointer(&m.data[0:4][0]))
	return int(ecode)
}

func (m *Message) IsError() bool {
	return m.GetErrorCode() != 0
}

func (m *Message) IsAck() bool {
	return m.Type == syscall.NLMSG_ERROR && m.GetErrorCode() == 0
}

func (m *Message) encode() []byte {
	len := syscall.NLMSG_HDRLEN + len(m.data)
	b := make([]byte, len)

	*(*uint32)(unsafe.Pointer(&b[0:4][0])) = uint32(len)
	*(*uint16)(unsafe.Pointer(&b[4:6][0])) = m.Type
	*(*uint16)(unsafe.Pointer(&b[6:8][0])) = m.Flags
	*(*uint32)(unsafe.Pointer(&b[8:12][0])) = m.Seq
	*(*uint32)(unsafe.Pointer(&b[12:16][0])) = m.Pid
	copy(b[16:], m.data)

	return b
}
