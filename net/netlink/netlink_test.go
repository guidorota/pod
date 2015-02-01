package netlink

import (
	"syscall"
	"testing"
)

func TestNetlinkMessage(t *testing.T) {
	d := make([]byte, 10)
	for i := range d {
		d[i] = byte(i)
	}

	msg := NewMessage(syscall.NLMSG_DONE,
		syscall.NLM_F_REQUEST|syscall.NLM_F_ACK, d)

	if msg == nil {
		t.Fatal("message creation failed")
	}
	if msg.Header.Type != syscall.NLMSG_DONE {
		t.Error("incorrect type")
	}
	if msg.Header.Flags != syscall.NLM_F_REQUEST|syscall.NLM_F_ACK {
		t.Error("incorrect flags")
	}

	if len(msg.Data) != len(d) {
		t.Fatal("different data slice length")
	}
	for i := range d {
		if d[i] != msg.Data[i] {
			t.Fatal("different data")
		}
	}
}
