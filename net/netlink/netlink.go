package netlink

import (
	"fmt"
	"os"
	"syscall"
)

type Connection struct {
	fd   int
	addr *syscall.SockaddrNetlink
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

// Send sends a netlink message to the destination address passed as parameter.
func (c *Connection) Send(dst *syscall.SockaddrNetlink, msg *Message) error {
	if msg.Pid == 0 {
		msg.Pid = c.addr.Pid
	}

	return syscall.Sendto(c.fd, msg.encode(), 0, dst)
}

// Recv receives and decodes netlink messages.  This function correctly handles
// and decodes multipart netlink messages.
func (c *Connection) Recv() ([]*Message, error) {
	msgs := []*Message{}
	b := make([]byte, os.Getpagesize())
	for {
		recvd, _, err := syscall.Recvfrom(c.fd, b, 0)
		if err != nil {
			return nil, err
		}
		ms, more, err := parseMessages(b[:recvd])
		if err != nil {
			return nil, err
		}
		msgs = append(msgs, ms...)
		if !more {
			break
		}
	}
	return msgs, nil
}

// parseMessages parses a slice of bytes read from a netlink socket and
// converts it into one or more netlink messages.  This function returns true
// if the decoded netlink messages indicate that there are more messages
// waiting to be read.
func parseMessages(b []byte) ([]*Message, bool, error) {
	more := false
	msgs := []*Message{}

	for len(b) > syscall.NLMSG_HDRLEN {
		msg, br, err := DecodeMessage(b)
		if err != nil {
			return nil, false, err
		}
		if msg.Type == syscall.NLMSG_DONE {
			return msgs, false, nil
		}

		msgs = append(msgs, msg)
		more = msg.Flags&syscall.NLM_F_MULTI != 0
		b = br
	}

	return msgs, more, nil
}

// Close closes the netlink connection
func (c *Connection) Close() error {
	return syscall.Close(c.fd)
}
