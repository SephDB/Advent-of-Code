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

type shape struct {
	values [3][3]bool
}

type problem struct {
	width  int
	height int
	counts []int
}

type parsed struct {
	shapes   []shape
	problems []problem
}

func ParseShape(scanner *bufio.Scanner) shape {
	res := shape{}
	for i := range 3 {
		scanner.Scan()
		line := scanner.Bytes()
		for j := range 3 {
			res.values[i][j] = line[j] == '#'
		}
	}
	scanner.Scan() //Empty line
	return res
}

func ParseProblem(line []byte) problem {
	size := bytes.IndexByte(line, ':')
	xindex := bytes.IndexByte(line[:size], 'x')
	width, _ := strconv.ParseInt(string(line[:xindex]), 10, 64)
	height, _ := strconv.ParseInt(string(line[xindex+1:size]), 10, 64)

	return problem{int(width), int(height), slices.Collect(Map(bytes.SplitSeq(line[size+2:], []byte{' '}), func(num []byte) int {
		val, _ := strconv.ParseInt(string(num), 10, 64)
		return int(val)
	}))}
}

func ParseInput(scanner *bufio.Scanner) parsed {
	res := parsed{}
	for scanner.Scan() {
		line := scanner.Bytes()

		if line[1] == ':' {
			res.shapes = append(res.shapes, ParseShape(scanner))
		} else {
			res.problems = append(res.problems, ParseProblem(line))
		}
	}

	return res
}

func Solve(input parsed) (int64, int64) {
	shape_sizes := make([]int, 0)
	for s := range slices.Values(input.shapes) {
		size := 0
		for y := range 3 {
			for x := range 3 {
				if s.values[y][x] {
					size++
				}
			}
		}
		shape_sizes = append(shape_sizes, size)
	}

	patently_unsolvable := 0

	for p := range slices.Values(input.problems) {
		required := 0
		for i := range p.counts {
			required += p.counts[i] * shape_sizes[i]
		}
		size := p.width * p.height
		if size < required {
			patently_unsolvable++
			fmt.Println("UNSOLVABLE")
		} else {
			fmt.Println(p.width*p.height, required)
		}
	}

	return int64(len(input.problems) - patently_unsolvable), 0
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)

	input := ParseInput(scanner)

	fmt.Println(input)

	fmt.Println(Solve(input))

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
