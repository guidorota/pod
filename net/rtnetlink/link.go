package rtnetlink

import (
	_ "syscall"

	_ "github.com/guidorota/pod/net/netlink"
)

type Ifinfomsg struct {
	Family uint16
	Type   uint16
	Index  int32
	Flags  uint32
	Change uint32
	Atts   []Rtattr
}
