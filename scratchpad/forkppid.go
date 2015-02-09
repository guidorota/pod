package main

import (
	"fmt"
	"syscall"
)

func main() {
	fmt.Println("parent:", syscall.Getpid())
	r1, _, err := syscall.Syscall6(syscall.SYS_CLONE, uintptr(syscall.SIGCHLD)|syscall.CLONE_NEWPID, 0, 0, 0, 0, 0)
	if err != 0 {
		fmt.Println("clone error:", err)
	}

	if r1 == 0 {
		fmt.Println("child:", syscall.Getpid())
	}
}
