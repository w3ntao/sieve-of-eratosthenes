package main

import (
	"fmt"
	"os"
	"strconv"
)

func sieve(read_in chan int, finish chan bool) {
	defer func() {
		finish <- true
	}()

	divisor := <-read_in
	fmt.Printf("prime %d\n", divisor)

	fork := false
	to_next_sieve := make(chan int)
	for {
		number, open := <-read_in
		if !open {
			close(to_next_sieve)
			return
		}

		if number%divisor == 0 {
			continue
		}

		if !fork {
			fork = true
			child_finish := make(chan bool)
			defer func() {
				_ = <-child_finish
			}()
			go sieve(to_next_sieve, child_finish)
		}
		to_next_sieve <- number
	}
}

func main() {
	if len(os.Args) != 2 {
		panic("primes.go expects 1 argument")
	}
	upper, _ := strconv.Atoi(os.Args[1])

	to_sieve := make(chan int)
	finish := make(chan bool)

	go sieve(to_sieve, finish)
	for number := 2; number < upper; number++ {
		to_sieve <- number
	}
	close(to_sieve)
	_ = <-finish
}
