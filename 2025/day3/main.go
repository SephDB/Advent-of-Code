package main

import (
	"bufio"
	"fmt"
	"os"
	"slices"
)

func biggest_integer(bank []byte, digits int, accum uint64) uint64 {
	if digits == 1 {
		return uint64(slices.Max(bank)-'0') + accum*10
	}
	first_digit := slices.Max(bank[:len(bank)-digits+1])
	first_index := slices.Index(bank, first_digit)
	return biggest_integer(bank[first_index+1:], digits-1, accum*10+uint64(first_digit-'0'))
}

func main() {
	var p1_answer uint64 = 0
	var p2_answer uint64 = 0

	scanner := bufio.NewScanner(os.Stdin)

	for scanner.Scan() {
		line := scanner.Bytes()

		p1_answer += biggest_integer(line, 2, 0)
		p2_answer += biggest_integer(line, 12, 0)
	}

	fmt.Println(p1_answer)
	fmt.Println(p2_answer)

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
