package netlink

import (
	"fmt"
	"syscall"
	"unsafe"
)

type Connection struct {
	fd   int
	addr *syscall.SockaddrNetlink
}

type Message struct {
	Type  uint16
	Flags uint16
	Seq   uint32
	Pid   uint32
	Data  []byte
}

func nlmsgAlign(len int) int {
	return (len + syscall.NLMSG_ALIGNTO - 1) & ^(syscall.NLMSG_ALIGNTO - 1)
}

func (m *Message) encode() []byte {
	len := syscall.NLMSG_HDRLEN + len(m.Data)
	b := make([]byte, len)

	*(*uint32)(unsafe.Pointer(&b[0:4][0])) = uint32(len)
	*(*uint16)(unsafe.Pointer(&b[4:6][0])) = m.Type
	*(*uint16)(unsafe.Pointer(&b[6:8][0])) = m.Flags
	*(*uint32)(unsafe.Pointer(&b[8:12][0])) = m.Seq
	*(*uint32)(unsafe.Pointer(&b[12:16][0])) = m.Pid
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

	c := &Connection{fd, addr}
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
	if msg.Pid == 0 {
		msg.Pid = uint32(syscall.Getpid())
	}
	return syscall.Sendto(c.fd, msg.encode(), 0, dst)
}

func (c *Connection) Recvfrom() (*Message, *syscall.SockaddrNetlink, error) {
	// TODO: implement
	return nil, nil, fmt.Errorf("not implemented")
}

// Close closes the netlink connection
func (c *Connection) Close() error {
	return syscall.Close(c.fd)
}
