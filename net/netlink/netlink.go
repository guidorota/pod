package netlink

import (
	"fmt"
	"syscall"
)

type Connection struct {
	fd   int
	seq  int
	addr *syscall.SockaddrNetlink
}

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

func (c *Connection) Close() error {
	return syscall.Close(c.fd)
}
