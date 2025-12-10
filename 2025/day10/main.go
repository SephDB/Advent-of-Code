package main

import (
	"bufio"
	"bytes"
	"fmt"
	"iter"
	"os"
	"slices"
	"strconv"
)

func Map[E, R any](input iter.Seq[E], mapper func(E) R) iter.Seq[R] {
	return func(yield func(R) bool) {
		for e := range input {
			if !yield(mapper(e)) {
				return
			}
		}
	}
}

type machine struct {
	target  uint16
	buttons []uint16
	jolts   []int
}

type parsed = []machine

func ParseInput(scanner *bufio.Scanner) parsed {
	res := make(parsed, 0)

	readTarget := func(input []byte) (uint16, []byte) {
		var res uint16 = 0
		start := bytes.IndexByte(input, '[')
		end := bytes.IndexByte(input, ']')
		target := input[start+1 : end]

		for i, b := range slices.All(target) {
			if b == '#' {
				res |= (1 << i)
			}
		}

		return res, input[end+1:]
	}

	readButtons := func(input []byte) ([]uint16, []byte) {
		res := make([]uint16, 0)

		for start := bytes.IndexByte(input, '('); start != -1; start = bytes.IndexByte(input, '(') {
			end := bytes.IndexByte(input, ')')

			var num uint16 = 0
			for i := start + 1; i < end; i += 2 {
				num |= (1 << (input[i] - '0'))
			}
			res = append(res, num)

			input = input[end+1:]
		}

		return res, input
	}

	readJoltage := func(input []byte) []int {
		input = input[bytes.IndexByte(input, '{')+1:]
		input = input[:bytes.IndexByte(input, '}')]
		return slices.Collect(Map(bytes.SplitSeq(input, []byte{','}), func(i []byte) int {
			value, _ := strconv.ParseInt(string(i), 10, 32)
			return int(value)
		}))
	}

	for scanner.Scan() {
		line := scanner.Bytes()
		target, line := readTarget(line)
		buttons, line := readButtons(line)

		res = append(res, machine{target, buttons, readJoltage(line)})
	}

	return res
}

func Part1CheckMachine(m machine) int {
	counts := make([]int, 1<<len(m.jolts))

	current := []uint16{0}
	next := make([]uint16, 0)
	for len(current) > 0 {
		next = next[:0] //Empty it out while keeping capacity

		for c := range slices.Values(current) {
			for b := range slices.Values(m.buttons) {
				n := c ^ b
				if counts[n] == 0 {
					counts[n] = counts[c] + 1
					if n == m.target {
						return counts[n]
					}
					next = append(next, n)
				}
			}
		}

		current, next = next, current
	}
	return -1
}

func Part1(input parsed) int64 {
	var result int64 = 0

	for m := range slices.Values(input) {
		ret := Part1CheckMachine(m)
		result += int64(ret)
	}

	return result
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)

	input := ParseInput(scanner)

	fmt.Println(Part1(input))

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
