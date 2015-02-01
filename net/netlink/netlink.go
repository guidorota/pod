package netlink

import (
	"fmt"
	"syscall"
	"unsafe"
)

type Connection struct {
	fd   int
	seq  int
	addr *syscall.SockaddrNetlink
}

type Header struct {
	Len   uint32
	Type  uint16
	Flags uint16
	Seq   uint32
	Pid   uint32
}

type Message struct {
	Header Header
	Data   []byte
}

func NewMessage(nl_type, nl_flags int, data []byte) *Message {
	m := new(Message)

	m.Header.Type = uint16(nl_type)
	m.Header.Flags = uint16(nl_flags)
	m.Header.Len = uint32(syscall.NLMSG_HDRLEN + len(data))

	m.Data = append(m.Data, data...)

	return m
}

func (m *Message) encode() []byte {
	b := make([]byte, m.Header.Len)

	*(*uint32)(unsafe.Pointer(&b[0:4][0])) = m.Header.Len
	*(*uint16)(unsafe.Pointer(&b[4:6][0])) = m.Header.Type
	*(*uint16)(unsafe.Pointer(&b[6:8][0])) = m.Header.Flags
	*(*uint32)(unsafe.Pointer(&b[8:12][0])) = m.Header.Seq
	*(*uint32)(unsafe.Pointer(&b[12:16][0])) = m.Header.Pid

	copy(b[16:], m.Data)

	return b
}

// Connect opens a netlink connection using the desired protocol
func Connect(protocol int) (*Connection, error) {
	fd, err := syscall.Socket(syscall.AF_NETLINK, syscall.SOCK_RAW, protocol)
	if err != nil {
		return nil, err
	}

	if err := setSockBuf(fd, 32768); err != nil {
		return nil, err
	}

	addr := &syscall.SockaddrNetlink{}
	addr.Family = syscall.AF_NETLINK
	if err := syscall.Bind(fd, addr); err != nil {
		return nil, err
	}

	addr, err = getLocalAddress(fd)
	if err != nil {
		return nil, err
	}

	c := &Connection{fd, 0, addr}
	return c, nil
}

func setSockBuf(fd, bufsize int) error {
	err := syscall.SetsockoptInt(fd, syscall.SOL_SOCKET,
		syscall.SO_SNDBUF, bufsize)
	if err != nil {
		return err
	}

	err = syscall.SetsockoptInt(fd, syscall.SOL_SOCKET,
		syscall.SO_RCVBUF, bufsize)
	if err != nil {
		return err
	}

	return nil
}

// getLocalAddress retrieves the address to which the socket descriptor has
// been bound. Returns an error in case the address doesn't turn out to be a
// valid netlink address.
func getLocalAddress(fd int) (*syscall.SockaddrNetlink, error) {
	addr, err := syscall.Getsockname(fd)
	if err != nil {
		return nil, err
	}

	nl_addr, ok := addr.(*syscall.SockaddrNetlink)
	if !ok {
		return nil, fmt.Errorf("not a netlink address")
	}

	return nl_addr, nil
}

func (c *Connection) Send(msg Message, dst syscall.SockaddrNetlink) error {
	// TODO: implement
	return fmt.Errorf("not implemented")
}

func (c *Connection) Recv() (*Message, *syscall.SockaddrNetlink, error) {
	// TODO: implement
	return nil, nil, fmt.Errorf("not implemented")
}

// Close closes the netlink connection
func (c *Connection) Close() error {
	return syscall.Close(c.fd)
}
