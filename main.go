package main

import (
	"fmt"
	"os"
	"syscall"

	"github.com/guidorota/pod/net/rtnetlink"
)

func main() {
	as, err := rtnetlink.GetAdds()
	if err != nil {
		fmt.Println("error fetching addresses:", err)
		os.Exit(1)
	}

	for _, a := range as {
		att := a.Atts.Get(syscall.IFA_ADDRESS)
		fmt.Println(a.Ifa.Index, att.AsIP())
	}

	os.Exit(0)
}
