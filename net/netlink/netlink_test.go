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
	l := syscall.NLMSG_HDRLEN + len(msg.Data)
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
		if data[i] != msg.Data[i] {
			t.Fatal("wrong data")
		}
	}
}

var err_msg = &Message{
	Type: syscall.NLMSG_ERROR,
	Data: []byte{0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
}

var sane_msg = &Message{
	Type: syscall.NLMSG_DONE,
	Data: make([]byte, 4),
}

var ack_msg = &Message{
	Type: syscall.NLMSG_ERROR,
	Data: make([]byte, 4),
}

func TestGetErrorCode(t *testing.T) {
	if ecode := err_msg.errorCode(); ecode != 10 {
		t.Error("wrong error code for error message", ecode)
	}

	if sane_msg.errorCode() != 0 {
		t.Error("normal message mistaken for an error")
	}

	if ack_msg.errorCode() != 0 {
		t.Error("ack message mistaken for an error")
	}
}

func TestGetError(t *testing.T) {
	err := err_msg.Error()
	if err == nil {
		t.Error("error message is not recognized as an error")
	}
	switch en := err.(type) {
	case syscall.Errno:
		if en != 10 {
			t.Error("wrong error code")
		}
	default:
		t.Error("wrong error type")
	}

	if sane_msg.Error() != nil {
		t.Error("normal message mistaken for an error")
	}

	if ack_msg.Error() != nil {
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

func TestEncodeMessage(t *testing.T) {
	b := err_msg.encode()

	l := Align(syscall.NLMSG_HDRLEN+8, NLMSG_ALIGNTO)
	if len(b) != l {
		t.Fatal("message encoded partially")
	}

	length := *(*uint32)(unsafe.Pointer(&b[0:4][0]))
	if length != syscall.NLMSG_HDRLEN+8 {
		t.Error("wrong length")
	}

	nl_type := *(*uint16)(unsafe.Pointer(&b[4:6][0]))
	if nl_type != err_msg.Type {
		t.Error("wrong type")
	}

	flags := *(*uint16)(unsafe.Pointer(&b[6:8][0]))
	if flags != err_msg.Flags {
		t.Error("wrong flags")
	}

	seq := *(*uint32)(unsafe.Pointer(&b[8:12][0]))
	if seq != err_msg.Seq {
		t.Error("wrong sequence")
	}

	pid := *(*uint32)(unsafe.Pointer(&b[12:16][0]))
	if pid != err_msg.Pid {
		t.Error("wrong pid")
	}

	ecode := *(*int32)(unsafe.Pointer(&b[16:20][0]))
	if int(-ecode) != err_msg.errorCode() {
		t.Error("wrong error code")
	}
}

func TestDecodeMessage(t *testing.T) {
	l := syscall.NLMSG_HDRLEN + 8
	al := Align(l, NLMSG_ALIGNTO)
	b := make([]byte, al)

	*(*uint32)(unsafe.Pointer(&b[0:4][0])) = uint32(l)
	*(*uint16)(unsafe.Pointer(&b[4:6][0])) = syscall.NLMSG_ERROR
	*(*uint16)(unsafe.Pointer(&b[6:8][0])) = 87
	*(*uint32)(unsafe.Pointer(&b[8:12][0])) = 12
	*(*uint32)(unsafe.Pointer(&b[12:16][0])) = 48
	copy(b[16:], err_msg.Data)

	msg, br, err := DecodeMessage(b)
	if err != nil {
		t.Fatal("decode error", err)
	}
	if len(br) != 0 {
		t.Error("message has not been decoded completely")
	}

	if msg.Type != syscall.NLMSG_ERROR {
		t.Error("wrong type")
	}
	if msg.Flags != 87 {
		t.Error("wrong flags")
	}
	if msg.Seq != 12 {
		t.Error("wrong sequence")
	}
	if msg.Pid != 48 {
		t.Error("wrong pid")
	}

	if msg.errorCode() != 10 {
		t.Error("wrong error code")
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
	msg.Flags = syscall.NLM_F_REQUEST | syscall.NLM_F_ACK
	msg.Seq = 1

	kernel := &syscall.SockaddrNetlink{}
	kernel.Family = syscall.AF_NETLINK
	kernel.Pid = 0

	if err := c.Send(kernel, msg); err != nil {
		t.Fatal("cannot send")
	}

	reply, err := c.Recv()
	if err != nil {
		t.Fatal("cannot receive")
	}
	if n_msg := len(reply); n_msg != 1 {
		t.Errorf("one message expected, received %v instead", n_msg)
	}
	if reply[0].IsError() {
		t.Fatal("error message received")
	}
	if !reply[0].IsAck() {
		t.Fatal("ack reply expected but not received")
	}
}
