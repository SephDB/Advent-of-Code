package main

import (
	"bufio"
	"fmt"
	"os"
	"regexp"
	"slices"
	"strconv"
	"strings"
)

func Map[S ~[]E, E, R any](slice S, mapper func(int, E) R) []R {
	mappedSlice := make([]R, len(slice))
	for i, v := range slice {
		mappedSlice[i] = mapper(i, v)
	}
	return mappedSlice
}

func Reduce[S ~[]E, E any](slice S, reduce func(E, E) E) E {
	accum := slice[0]
	for v := range slices.Values(slice[1:]) {
		accum = reduce(accum, v)
	}
	return accum
}

func Part1(lines []string) int64 {
	var result int64 = 0

	type Problem struct {
		add int64
		mul int64
	}
	var numberElement = regexp.MustCompile(`\d+`)
	var opElement = regexp.MustCompile(`[*+]`)

	p := Map(numberElement.FindAllString(lines[0], -1), func(_ int, num string) Problem {
		value, _ := strconv.ParseInt(num, 10, 64)
		return Problem{value, value}
	})

	for line := range slices.Values(lines[1 : len(lines)-1]) {
		numbers := numberElement.FindAllString(line, -1)
		for i, num := range slices.All(numbers) {
			value, _ := strconv.ParseInt(num, 10, 64)
			p[i].add += value
			p[i].mul *= value
		}
	}

	ops := opElement.FindAllString(lines[len(lines)-1], -1)
	for i, op := range slices.All(ops) {
		if op == "+" {
			result += p[i].add
		} else {
			result += p[i].mul
		}
	}

	return result
}

func Part2_parseblock(block []string) []int64 {
	result := make([]int64, 0, len(block[0]))
	for i := range block[0] {
		s := string(Map(block, func(_ int, line string) byte {
			return line[i]
		}))
		s = strings.TrimSpace(s)
		value, err := strconv.ParseInt(s, 10, 64)
		if err == nil {
			result = append(result, value)
		}
	}
	return result
}

func Part2(lines []string) int64 {
	var result int64 = 0

	opstring := lines[len(lines)-1]
	var opElement = regexp.MustCompile(`[*+] +`)
	ops := opElement.FindAllStringIndex(opstring, -1)

	lines = lines[:len(lines)-1]

	for op := range slices.Values(ops) {
		block := Part2_parseblock(Map(lines, func(_ int, line string) string {
			return line[op[0]:op[1]]
		}))

		if opstring[op[0]] == '+' {
			result += Reduce(block, func(a int64, b int64) int64 {
				return a + b
			})
		} else {
			result += Reduce(block, func(a int64, b int64) int64 {
				return a * b
			})
		}
	}

	return result
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)

	lines := make([]string, 0)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}

	fmt.Println(Part1(lines))
	fmt.Println(Part2(lines))

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
