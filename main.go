package main

import (
	"fmt"
	"syscall"
	"unsafe"

	"github.com/guidorota/pod/net/netlink"
)

func main() {
	fmt.Println("netlink test")
	c, err := netlink.Connect(syscall.NETLINK_ROUTE)
	if err != nil {
		return
	}
	defer c.Close()

	msg := &netlink.Message{}
	msg.Type = syscall.RTM_GETLINK
	msg.Flags = syscall.NLM_F_DUMP | syscall.NLM_F_REQUEST
	msg.Seq = 1
	msg.Data = make([]byte, syscall.SizeofIfInfomsg)
	*(*uint16)(unsafe.Pointer(&msg.Data[0:2][0])) = syscall.AF_UNSPEC

	kernel := &syscall.SockaddrNetlink{}
	kernel.Family = syscall.AF_NETLINK
	kernel.Pid = 0

	err = c.Sendto(kernel, msg)
	if err != nil {
		fmt.Println("send error:", err)
		return
	}

	msgs, err := c.Recvfrom()
	if err != nil {
		fmt.Println("recv error:", err)
		return
	}
	fmt.Println("messages received:", len(msgs))

	fmt.Println("sucess")
}
