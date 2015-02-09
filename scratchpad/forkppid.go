package main

import (
	"fmt"
	"os"
	"os/exec"
	"syscall"
)

func main() {
	fmt.Println("parent:", syscall.Getpid())

	c := exec.Command("./ppid", "")
	c.Stdout = os.Stdout
	c.SysProcAttr = &syscall.SysProcAttr{}
	c.SysProcAttr.Cloneflags = syscall.CLONE_NEWPID

	if err := c.Run(); err != nil {
		fmt.Println("error running ppid:", err)
		return
	}
}
