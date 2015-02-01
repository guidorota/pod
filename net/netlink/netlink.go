package netlink

import (
	"fmt"
	"syscall"
	"unsafe"
)

type Connection struct {
	fd   int
	seq  uint32
	addr *syscall.SockaddrNetlink
}

type Message struct {
	Type  uint16
	Flags uint16
	Seq   uint32
	Data  [][]byte
}

func (c *Connection) NewMessage(nl_type, nl_flags int, data ...[]byte) *Message {
	m := new(Message)

	m.Type = uint16(nl_type)
	m.Seq = c.seq
	c.seq++
	m.Flags = uint16(nl_flags)

	m.Data = make([][]byte, len(data))
	for i, d := range data {
		m.Data[i] = append(m.Data[i], d...)
	}

	return m
}

func (m *Message) space() int {
	l := 0

	for _, d := range m.Data {
		l += nlmsgSpace(len(d))
	}

	return l
}

func nlmsgSpace(len int) int {
	return nlmsgAlign(syscall.NLMSG_HDRLEN + len)
}

func nlmsgAlign(len int) int {
	return (len + syscall.NLMSG_ALIGNTO - 1) & ^(syscall.NLMSG_ALIGNTO - 1)
}

func (m *Message) encode(pid uint32) []byte {
	b := make([]byte, m.space())

	i := 0
	for _, d := range m.Data {
		l := syscall.NLMSG_HDRLEN + len(d)
		*(*uint32)(unsafe.Pointer(&b[i+0 : i+4][0])) = uint32(l)
		*(*uint16)(unsafe.Pointer(&b[i+4 : i+6][0])) = m.Type
		*(*uint16)(unsafe.Pointer(&b[i+6 : i+8][0])) = m.Flags
		*(*uint32)(unsafe.Pointer(&b[i+8 : i+12][0])) = m.Seq
		*(*uint32)(unsafe.Pointer(&b[i+12 : i+16][0])) = pid
		copy(b[i+16:], d)
		i += nlmsgSpace(len(d))
	}

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

func (c *Connection) Sendto(dst *syscall.SockaddrNetlink, msg *Message) error {
	b := msg.encode(uint32(syscall.Getpid()))
	return syscall.Sendto(c.fd, b, 0, dst)
}

func (c *Connection) Recvfrom() (*Message, *syscall.SockaddrNetlink, error) {
	// TODO: implement
	return nil, nil, fmt.Errorf("not implemented")
}

// Close closes the netlink connection
func (c *Connection) Close() error {
	return syscall.Close(c.fd)
}
