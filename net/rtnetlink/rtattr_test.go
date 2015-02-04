package rtnetlink

import (
	"testing"
	"unsafe"
)

func TestUint8Attr(t *testing.T) {
	a := NewUint8Attr(5, 12)

	if len(a.Data) != 1 {
		t.Error("wrong data lenght")
	}
	v := *(*uint8)(unsafe.Pointer(&a.Data[0]))
	if v != 12 {
		t.Error("wrong data content")
	}

	if a.Type != 5 {
		t.Error("wrong type")
	}
	if a.AsUint8() != 12 {
		t.Error("wrong value")
	}

	dec, b, err := DecodeAttribute(a.Encode())
	if err != nil {
		t.Error("decode error")
	}
	if len(b) != 0 {
		t.Error("attribute was not decoded completely")
	}
	if a.Type != dec.Type ||
		a.AsUint8() != dec.AsUint8() {
		t.Error("encode/decode error")
	}
}
