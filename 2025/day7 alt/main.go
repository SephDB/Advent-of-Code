package main

import (
	"bufio"
	"fmt"
	"os"
	"slices"
)

type parsed struct {
	start     int
	width     int
	splitters [][]int
}

func Solve(input parsed) (int64, int64) {
	beams := make([]int64, input.width)
	beams[input.start] = 1

	var part1_res int64 = 0

	for splitters := range slices.Values(input.splitters) {
		for splitter := range slices.Values(splitters) {
			if beams[splitter] > 0 {
				beams[splitter-1] += beams[splitter]
				beams[splitter+1] += beams[splitter]
				beams[splitter] = 0
				part1_res++
			}
		}
	}

	var part2_res int64 = 0
	for path_count := range slices.Values(beams) {
		part2_res += path_count
	}

	return part1_res, part2_res
}

func ParseInput(scanner *bufio.Scanner) parsed {
	res := make([][]int, 0)

	scanner.Scan()
	start := slices.Index(scanner.Bytes(), 'S')
	width := len(scanner.Bytes())

	for scanner.Scan() {
		line := scanner.Bytes()
		splitters := make([]int, 0)

		for i, c := range slices.All(line) {
			if c == '^' {
				splitters = append(splitters, i)
			}
		}

		res = append(res, splitters)
	}

	return parsed{start, width, res}
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)

	input := ParseInput(scanner)

	part1, part2 := Solve(input)

	fmt.Println(part1)
	fmt.Println(part2)

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
