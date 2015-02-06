package main

import (
	"fmt"
	"os"

	"github.com/guidorota/pod/net/rtnetlink"
)

func main() {
	as, err := rtnetlink.GetAdds()
	if err != nil {
		fmt.Println("error fetching addresses:", err)
		os.Exit(1)
	}

	for _, a := range as {
		fmt.Println(a.Ifa.Index, a.Ifa.PrefixLen)
	}
	os.Exit(0)
}
