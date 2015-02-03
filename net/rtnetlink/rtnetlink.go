package rtnetlink

import (
	"syscall"

	"github.com/guidorota/pod/net/netlink"
)

// kernel is the netlink address for the Linux kernel
var kernel = &syscall.SockaddrNetlink{
	Family: syscall.AF_NETLINK,
	Pid:    0,
}

// request is a utility method that opens a NETLINK_ROUTE connection, sends a
// single request message and waits for the related response.
func request(msg *netlink.Message) ([]*netlink.Message, error) {
	c, err := netlink.Connect(syscall.NETLINK_ROUTE)
	if err != nil {
		return nil, err
	}
	defer c.Close()

	if err := c.Send(kernel, msg); err != nil {
		return nil, err
	}

	msgs, err := c.Recv()
	if err != nil {
		return nil, err
	}

	return msgs, nil
}
