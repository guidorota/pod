package netlink

import (
	"syscall"
	"testing"
	"unsafe"
)

func createMessage(nl_type, nl_flags int) *Message {
	d := make([]byte, 10)
	for i := range d {
		d[i] = byte(i)
	}

	return NewMessage(nl_type, nl_flags, d)
}

func TestMessageCreation(t *testing.T) {
	msg := createMessage(syscall.NLMSG_DONE,
		syscall.NLM_F_REQUEST|syscall.NLM_F_ACK)

	if msg == nil {
		t.Fatal("message creation failed")
	}
	if msg.Header.Type != syscall.NLMSG_DONE {
		t.Error("incorrect type")
	}
	if msg.Header.Flags != syscall.NLM_F_REQUEST|syscall.NLM_F_ACK {
		t.Error("incorrect flags")
	}

	if len(msg.Data) != 10 {
		t.Fatal("different data slice length")
	}
	for i := range msg.Data {
		if msg.Data[i] != byte(i) {
			t.Fatal("different data")
		}
	}
}

func TestMessageEncode(t *testing.T) {
	msg := createMessage(syscall.NLMSG_DONE,
		syscall.NLM_F_REQUEST|syscall.NLM_F_ACK)
	msg.Header.Seq = 10
	msg.Header.Pid = 2586

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
