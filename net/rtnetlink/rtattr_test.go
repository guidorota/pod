package rtnetlink

import (
	"testing"
	"unsafe"
)

func TestUint8Attr(t *testing.T) {
	value := uint8(12)
	a := NewUint8Attr(5, value)

	if len(a.Data) != 1 {
		t.Error("wrong data lenght")
	}
	v := *(*uint8)(unsafe.Pointer(&a.Data[0]))
	if v != value {
		t.Error("wrong data content")
	}

	if a.Type != 5 {
		t.Error("wrong type")
	}
	if a.AsUint8() != value {
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

func TestInt8Attr(t *testing.T) {
	value := int8(-12)
	a := NewInt8Attr(5, value)

	if len(a.Data) != 1 {
		t.Error("wrong data lenght")
	}
	v := *(*int8)(unsafe.Pointer(&a.Data[0]))
	if v != value {
		t.Error("wrong data content")
	}

	if a.Type != 5 {
		t.Error("wrong type")
	}
	if a.AsInt8() != value {
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
		a.AsInt8() != dec.AsInt8() {
		t.Error("encode/decode error")
	}
}

func TestUint16Attr(t *testing.T) {
	value := uint16(847)
	a := NewUint16Attr(5, value)

	if len(a.Data) != 2 {
		t.Error("wrong data lenght")
	}
	v := *(*uint16)(unsafe.Pointer(&a.Data[0:2][0]))
	if v != value {
		t.Error("wrong data content")
	}

	if a.Type != 5 {
		t.Error("wrong type")
	}
	if a.AsUint16() != value {
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
		a.AsUint16() != dec.AsUint16() {
		t.Error("encode/decode error")
	}
}

func TestInt16Attr(t *testing.T) {
	value := int16(-847)
	a := NewInt16Attr(5, value)

	if len(a.Data) != 2 {
		t.Error("wrong data lenght")
	}
	v := *(*int16)(unsafe.Pointer(&a.Data[0:2][0]))
	if v != value {
		t.Error("wrong data content")
	}

	if a.Type != 5 {
		t.Error("wrong type")
	}
	if a.AsInt16() != value {
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
		a.AsInt16() != dec.AsInt16() {
		t.Error("encode/decode error")
	}
}

func TestUint32Attr(t *testing.T) {
	value := uint32(67836)
	a := NewUint32Attr(5, value)

	if len(a.Data) != 4 {
		t.Error("wrong data lenght")
	}
	v := *(*uint32)(unsafe.Pointer(&a.Data[0:4][0]))
	if v != value {
		t.Error("wrong data content")
	}

	if a.Type != 5 {
		t.Error("wrong type")
	}
	if a.AsUint32() != value {
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
		a.AsUint32() != dec.AsUint32() {
		t.Error("encode/decode error")
	}
}

func TestInt32Attr(t *testing.T) {
	value := int32(67836)
	a := NewInt32Attr(5, value)

	if len(a.Data) != 4 {
		t.Error("wrong data lenght")
	}
	v := *(*int32)(unsafe.Pointer(&a.Data[0:4][0]))
	if v != value {
		t.Error("wrong data content")
	}

	if a.Type != 5 {
		t.Error("wrong type")
	}
	if a.AsInt32() != value {
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
		a.AsInt32() != dec.AsInt32() {
		t.Error("encode/decode error")
	}
}

func TestStringAttr(t *testing.T) {
	value := "test_string"
	a := NewStringAttr(5, value)

	if len(a.Data) != 11 {
		t.Error("wrong data lenght", len(a.Data))
	}
	v := string(a.Data)
	if v != value {
		t.Error("wrong data content")
	}

	if a.Type != 5 {
		t.Error("wrong type")
	}
	if a.AsString() != value {
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
		a.AsString() != dec.AsString() {
		t.Error("encode/decode error")
	}
}
