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

func TestNlmsgAlign(t *testing.T) {
	al := syscall.NLMSG_ALIGNTO
	for i := 0; i < 100; i++ {
		c := i + (al-(i%al))%al
		if nlmsgAlign(i) != c {
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
