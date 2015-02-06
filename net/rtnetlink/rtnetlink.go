package rtnetlink

import (
	"fmt"
	"syscall"

	"github.com/guidorota/pod/net/netlink"
)

// kernel is the netlink address for the Linux kernel
var kernel = &syscall.SockaddrNetlink{
	Family: syscall.AF_NETLINK,
	Pid:    0,
}

// requestAck is a utility method that opens a NETLINK_ROUTE connection, sends
// a single request message and checks that a single ack is returned.
func requestAck(req *netlink.Message) error {
	msgs, err := request(req)
	if err != nil {
		return err
	}
	if len(msgs) != 1 {
		return fmt.Errorf("unexpected number of response messages")
	}

	m := msgs[0]
	if !m.IsAck() {
		return fmt.Errorf("no ack received")
	}

	return nil
}

// request is a utility method that opens a NETLINK_ROUTE connection, sends a
// single request message and waits for the related response.
func request(req *netlink.Message) ([]*netlink.Message, error) {
	c, err := netlink.Connect(syscall.NETLINK_ROUTE)
	if err != nil {
		return nil, err
	}
	defer c.Close()

	if err := c.Send(kernel, req); err != nil {
		return nil, err
	}

	msgs, err := c.Recv()
	if err != nil {
		return nil, err
	}

	// Check if there's been an error in the rtnetlink invocation
	if len(msgs) == 1 {
		m := msgs[0]
		if m.IsError() {
			return nil, m.Error()
		}
	}

	return msgs, nil
}
