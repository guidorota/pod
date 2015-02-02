package netlink

import (
	"syscall"
	"testing"
	"unsafe"
)

var data = []byte{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

const seq = 10
const pid = 2034

var msg = &Message{
	syscall.NLMSG_DONE,
	syscall.NLM_F_REQUEST | syscall.NLM_F_ACK,
	seq,
	pid,
	data,
}

func TestOpenConnection(t *testing.T) {
	c, err := Connect(syscall.NETLINK_ROUTE)
	if err != nil {
		t.Fatal("connection error:", err)
	}
	c.Close()
}

func TestAlign(t *testing.T) {
	al := syscall.NLMSG_ALIGNTO
	for i := 0; i < 100; i++ {
		c := i + (al-(i%al))%al
		if Align(i, al) != c {
			t.Fatal("wrong alignment")
		}
	}
}

func TestMessageEncode(t *testing.T) {
	b := msg.encode()
	l := syscall.NLMSG_HDRLEN + len(msg.data)
	if len(b) < l {
		t.Fatal("slice too short")
	}

	if *(*uint32)(unsafe.Pointer(&b[0:4][0])) != uint32(l) {
		t.Error("wrong length")
	}
	if *(*uint16)(unsafe.Pointer(&b[4:6][0])) != msg.Type {
		t.Error("wrong type")
	}
	if *(*uint16)(unsafe.Pointer(&b[6:8][0])) != msg.Flags {
		t.Error("wrong flags")
	}
	if *(*uint32)(unsafe.Pointer(&b[8:12][0])) != msg.Seq {
		t.Error("wrong seq")
	}
	if *(*uint32)(unsafe.Pointer(&b[12:16][0])) != msg.Pid {
		t.Error("wrong pid")
	}

	data := b[16:]
	for i := range data {
		if data[i] != msg.data[i] {
			t.Fatal("wrong data")
		}
	}
}

var err_msg = &Message{
	Type: syscall.NLMSG_ERROR,
	data: []byte{0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
}

var sane_msg = &Message{
	Type: syscall.NLMSG_DONE,
	data: make([]byte, 4),
}

var ack_msg = &Message{
	Type: syscall.NLMSG_ERROR,
	data: make([]byte, 4),
}

func TestGetErrorCode(t *testing.T) {
	if err_msg.GetErrorCode() != -10 {
		t.Error("wrong error code for error message", err_msg.GetErrorCode())
	}

	if sane_msg.GetErrorCode() != 0 {
		t.Error("normal message mistaken for an error")
	}

	if ack_msg.GetErrorCode() != 0 {
		t.Error("ack message mistaken for an error")
	}
}

func TestIsError(t *testing.T) {
	if !err_msg.IsError() {
		t.Error("error message not recognized")
	}

	if sane_msg.IsError() {
		t.Error("normal message mistaken for an error")
	}

	if ack_msg.IsError() {
		t.Error("ack message mistaken for an error")
	}
}

func TestIsAck(t *testing.T) {
	if err_msg.IsAck() {
		t.Error("error message mistaken for acknowledgement")
	}

	if sane_msg.IsAck() {
		t.Error("normal message mistaken for an acknowledgement")
	}

	if !ack_msg.IsAck() {
		t.Error("ack message not recognized")
	}
}

func TestCommunication(t *testing.T) {
	c, err := Connect(syscall.NETLINK_ROUTE)
	if err != nil {
		t.Fatal("cannot connect")
	}
	defer c.Close()

	msg := &Message{}
	msg.Type = syscall.RTM_GETLINK
	msg.Flags = syscall.NLM_F_DUMP | syscall.NLM_F_REQUEST
	msg.Seq = 1
	msg.data = make([]byte, syscall.SizeofIfInfomsg)
	*(*uint16)(unsafe.Pointer(&msg.data[0:2][0])) = syscall.AF_UNSPEC

	kernel := &syscall.SockaddrNetlink{}
	kernel.Family = syscall.AF_NETLINK
	kernel.Pid = 0

	if err := c.Send(kernel, msg); err != nil {
		t.Fatal("cannot send")
	}

	msgs, err := c.Recv()
	if err != nil {
		t.Fatal("cannot receive")
	}
	if len(msgs) == 0 {
		t.Error("no message received")
	}
}
