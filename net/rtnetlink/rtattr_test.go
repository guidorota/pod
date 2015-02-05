package rtnetlink

import (
	"testing"
	"unsafe"
)

func TestUint8Attr(t *testing.T) {
	value := uint8(12)
	a := NewUint8Attr(5, value)

	if len(a.data) != 1 {
		t.Error("wrong data lenght")
	}
	v := *(*uint8)(unsafe.Pointer(&a.data[0]))
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

	if len(a.data) != 1 {
		t.Error("wrong data lenght")
	}
	v := *(*int8)(unsafe.Pointer(&a.data[0]))
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

	if len(a.data) != 2 {
		t.Error("wrong data lenght")
	}
	v := *(*uint16)(unsafe.Pointer(&a.data[0:2][0]))
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

	if len(a.data) != 2 {
		t.Error("wrong data lenght")
	}
	v := *(*int16)(unsafe.Pointer(&a.data[0:2][0]))
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

	if len(a.data) != 4 {
		t.Error("wrong data lenght")
	}
	v := *(*uint32)(unsafe.Pointer(&a.data[0:4][0]))
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

	if len(a.data) != 4 {
		t.Error("wrong data lenght")
	}
	v := *(*int32)(unsafe.Pointer(&a.data[0:4][0]))
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

	if len(a.data) != 11 {
		t.Error("wrong data lenght", len(a.data))
	}
	v := string(a.data)
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

func TestAttributeListAddGet(t *testing.T) {
	al := NewAttributeList()

	att1 := NewUint16Attr(12, 43)
	att2 := NewStringAttr(43, "test")

	al.Add(att1)
	al.Add(att2)

	g1 := al.Get(12)
	if g1 == nil {
		t.Error("missing attribute")
	}
	if g1 != att1 {
		t.Error("wrong attribute")
	}

	g2 := al.Get(43)
	if g2 == nil {
		t.Error("missing attribute")
	}
	if g2 != att2 {
		t.Error("wrong attribute")
	}

	miss := al.Get(90)
	if miss != nil {
		t.Error("attribute should not exist")
	}

	att3 := NewInt32Attr(12, -23)
	al.Add(att3)
	g3 := al.Get(12)
	if g3 == nil {
		t.Error("missing attribute")
	}
	if g3 != att3 {
		t.Error("wrong attribute")
	}
}

func TestAttributeListEncodeDecode(t *testing.T) {
	al := NewAttributeList()

	att1 := NewUint16Attr(12, 43)
	al.Add(att1)
	att2 := NewStringAttr(43, "test")
	al.Add(att2)
	att3 := NewInt32Attr(56, -45)
	al.Add(att3)

	dec, b, err := DecodeAttributeList(al.Encode())
	if err != nil {
		t.Fatal("error decoding attribute list")
	}
	if len(b) > 0 {
		t.Error("attribute list partially decoded")
	}

	g1 := dec.Get(int(att1.Type))
	if g1 == nil {
		t.Fatal("missing attribute")
	}
	if g1.Type != att1.Type ||
		g1.AsUint16() != att1.AsUint16() {
		t.Error("wrong attribute")
	}

	g2 := dec.Get(int(att2.Type))
	if g2 == nil {
		t.Fatal("missing attribute")
	}
	if g2.Type != att2.Type ||
		g2.AsString() != att2.AsString() {
		t.Error("wrong attribute")
	}

	g3 := dec.Get(int(att3.Type))
	if g3 == nil {
		t.Fatal("missing attribute")
	}
	if g3.Type != att3.Type ||
		g3.AsInt32() != att3.AsInt32() {
		t.Error("wrong attribute")
	}
}
