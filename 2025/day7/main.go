package main

import (
	"bufio"
	"fmt"
	"os"
	"slices"
)

func MergeFunc[S ~[]E, E any](s S, eq func(E, E) bool, merge func(E, E) E) S {
	if len(s) < 2 {
		return s
	}
	for k := 1; k < len(s); k++ {
		if eq(s[k-1], s[k]) {
			s[k-1] = merge(s[k-1], s[k])
			s2 := s[k:]
			for k2 := 1; k2 < len(s2); k2++ {
				if !eq(s[k-1], s2[k2]) {
					s[k] = s2[k2]
					k++
				} else {
					s[k-1] = merge(s[k-1], s2[k2])
				}
			}

			clear(s[k:]) // zero/nil out the obsolete elements, for GC
			return s[:k]
		}
	}
	return s
}

type parsed struct {
	start     int
	splitters [][]int
}

func Part1(input parsed) int64 {
	var result int64 = 0

	current := []int{input.start}

	for splitters := range slices.Values(input.splitters) {
		if len(splitters) == 0 {
			continue
		}
		next := make([]int, 0, len(current)+len(splitters))
		splitter_index := 0
		for beam := range slices.Values(current) {
			for splitter_index < len(splitters) && beam > splitters[splitter_index] {
				splitter_index++
			}
			if splitter_index >= len(splitters) {
				next = append(next, beam)
				continue
			}
			if beam == splitters[splitter_index] {
				next = append(next, beam-1, beam+1)
				result++
			} else {
				next = append(next, beam)
			}
		}
		current = slices.Compact(next)
	}

	return result
}

func Part2(input parsed) int64 {
	var result int64 = 0

	type paths struct {
		column int
		count  int
	}
	current := []paths{{input.start, 1}}

	for splitters := range slices.Values(input.splitters) {
		if len(splitters) == 0 {
			continue
		}
		next := make([]paths, 0, len(current)+len(splitters))
		splitter_index := 0
		for beam := range slices.Values(current) {
			for splitter_index < len(splitters) && beam.column > splitters[splitter_index] {
				splitter_index++
			}
			if splitter_index >= len(splitters) {
				next = append(next, beam)
				continue
			}
			if beam.column == splitters[splitter_index] {
				next = append(next, paths{beam.column - 1, beam.count}, paths{beam.column + 1, beam.count})
			} else {
				next = append(next, beam)
			}
		}
		current = MergeFunc(next, func(a paths, b paths) bool {
			return a.column == b.column
		}, func(a paths, b paths) paths {
			return paths{a.column, a.count + b.count}
		})
	}

	for path := range slices.Values(current) {
		result += int64(path.count)
	}

	return result
}

func ParseInput(scanner *bufio.Scanner) parsed {
	res := make([][]int, 0)

	scanner.Scan()
	start := slices.Index(scanner.Bytes(), 'S')

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

	return parsed{start, res}
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)

	input := ParseInput(scanner)

	fmt.Println(Part1(input))
	fmt.Println(Part2(input))

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
