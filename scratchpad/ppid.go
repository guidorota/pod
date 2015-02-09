package main

import (
	"fmt"
	"syscall"
)

func main() {
	fmt.Println("child:", syscall.Getpid())
}
