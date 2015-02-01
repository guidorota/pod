package netlink

import (
	"syscall"
	"testing"
	_ "unsafe"
)

var data = []byte{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

func TestOpenConnection(t *testing.T) {
	c, err := Connect(syscall.NETLINK_ROUTE)
	if err != nil {
		t.Fatal("connection error:", err)
	}
	if c.seq != 0 {
		t.Error("wrong initial value for connection sequence")
	}
	c.Close()
}

func TestNlmsgAlign(t *testing.T) {
	al := syscall.NLMSG_ALIGNTO
	for i := 0; i < 100; i++ {
		a := nlmsgAlign(i)
		c := i + (al-(i%al))%al
		if a != c {
			t.Fatal("wrong alignment", i, a, c)
		}
	}
}

// newMessage is a utility function for creating test messages
func newMessage(nl_type, nl_flags int, data ...[]byte) *Message {
	c, err := Connect(syscall.NETLINK_ROUTE)
	if err != nil {
		return nil
	}
	m := c.NewMessage(nl_type, nl_flags, data...)
	c.Close()
	return m
}

func TestMessageCreation(t *testing.T) {
	msg := newMessage(syscall.NLMSG_DONE,
		syscall.NLM_F_REQUEST|syscall.NLM_F_ACK, data, data)

	if msg == nil {
		t.Fatal("message creation failed")
	}
	if msg.Type != syscall.NLMSG_DONE {
		t.Error("incorrect type")
	}
	if msg.Flags != syscall.NLM_F_REQUEST|syscall.NLM_F_ACK {
		t.Error("incorrect flags")
	}
	if msg.Seq != 0 {
		t.Error("incorrect seq")
	}

	if len(msg.Data) != 2 {
		t.Fatal("different data slice length")
	}
	for _, d := range msg.Data {
		if len(d) != 10 {
			t.Fatal("different data length in slice 0")
		}
		for i := range d {
			if d[i] != byte(i) {
				t.Fatal("different data")
			}
		}
	}
}

/*
func TestMessageEncode(t *testing.T) {
	msg := createMessage(syscall.NLMSG_DONE,
		syscall.NLM_F_REQUEST|syscall.NLM_F_ACK)

	b := msg.encode()
	if uint32(len(b)) < msg.Header.Len {
		t.Fatal("slice too short")
	}

	if *(*uint32)(unsafe.Pointer(&b[0:4][0])) != msg.Header.Len {
		t.Error("wrong length")
	}
	if *(*uint16)(unsafe.Pointer(&b[4:6][0])) != msg.Header.Type {
		t.Error("wrong type")
	}
	if *(*uint16)(unsafe.Pointer(&b[6:8][0])) != msg.Header.Flags {
		t.Error("wrong flags")
	}
	if *(*uint32)(unsafe.Pointer(&b[8:12][0])) != msg.Header.Seq {
		t.Error("wrong seq")
	}
	if *(*uint32)(unsafe.Pointer(&b[12:16][0])) != msg.Header.Pid {
		t.Error("wrong pid")
	}

	data := b[16:]
	for i := range data {
		if data[i] != msg.Data[i] {
			t.Fatal("wrong data")
		}
	}
}
*/
