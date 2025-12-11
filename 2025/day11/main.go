package main

import (
	"bufio"
	"bytes"
	"fmt"
	"iter"
	"os"
	"slices"
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

type parsed struct {
	you         int
	out         int
	svr         int
	dac         int
	fft         int
	connections [][]int
}

func ParseInput(scanner *bufio.Scanner) parsed {
	conns := make([][]int, 0)
	name_lookup := make(map[int]int, 0)

	name_to_num := func(name []byte) int {
		return int(name[0]-'a') + 26*(int(name[1]-'a')+26*int(name[2]-'a'))
	}

	for scanner.Scan() {
		line := scanner.Bytes()
		name := line[:bytes.IndexByte(line, ':')]

		name_lookup[name_to_num(name)] = len(conns)

		line = line[5:]

		conns = append(conns, slices.Collect(Map(bytes.SplitSeq(line, []byte{' '}), name_to_num)))
	}
	name_lookup[name_to_num([]byte("out"))] = len(conns)
	conns = append(conns, []int{}) //Empty entry for out

	//Translate name numbers to actual indices
	for connections := range slices.Values(conns) {
		for i := range connections {
			connections[i] = name_lookup[connections[i]]
		}
	}

	name := func(a string) int {
		return name_lookup[name_to_num([]byte(a))]
	}

	return parsed{name("you"), name("out"), name("svr"), name("dac"), name("fft"), conns}
}

type pathcounts struct {
	out  int64
	both int64
	dac  int64
	fft  int64
}

func (p *pathcounts) Add(p2 pathcounts) {
	p.out += p2.out
	p.both += p2.both
	p.dac += p2.dac
	p.fft += p2.fft
}

func Solve(input parsed) (int64, int64) {
	//Simple DFS with memoization

	paths := make([]pathcounts, len(input.connections))

	var calc func(int) pathcounts

	calc = func(node int) pathcounts {
		if paths[node].out != 0 {
			return paths[node]
		}
		if node == input.out {
			paths[node].out = 1
			return paths[node]
		}

		result := &paths[node]
		for n := range slices.Values(input.connections[node]) {
			result.Add(calc(n))
		}
		if node == input.dac {
			result.dac = result.out
			if result.fft > 0 {
				result.both = result.fft
			}
		}
		if node == input.fft {
			result.fft = result.out
			if result.dac > 0 {
				result.both = result.dac
			}
		}
		return *result
	}

	return calc(input.you).out, calc(input.svr).both
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)

	input := ParseInput(scanner)

	fmt.Println(Solve(input))

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
