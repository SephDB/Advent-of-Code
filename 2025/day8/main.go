package main

import (
	"bufio"
	"bytes"
	"cmp"
	"fmt"
	"os"
	"slices"
	"strconv"
)

func Map[S ~[]E, E, R any](slice S, mapper func(int, E) R) []R {
	mappedSlice := make([]R, len(slice))
	for i, v := range slice {
		mappedSlice[i] = mapper(i, v)
	}
	return mappedSlice
}

type coord struct {
	X int64
	Y int64
	Z int64
}

type parsed []coord

type DisjointSet []struct {
	parent int
	size   int
}

func DisjointSet_Create(size int) DisjointSet {
	ret := make(DisjointSet, size)
	for i := range ret {
		ret[i].parent = i
		ret[i].size = 1
	}
	return ret
}

func (s DisjointSet) FindSet(parent int) int {
	for s[parent].parent != parent {
		grandparent := s[s[parent].parent].parent
		s[parent].parent = grandparent
		parent = grandparent
	}
	return parent
}

func (s DisjointSet) Union(a int, b int) bool {
	a = s.FindSet(a)
	b = s.FindSet(b)
	if a == b {
		return false
	}

	if s[a].size < s[b].size {
		a, b = b, a
	}

	s[b].parent = a
	s[a].size += s[b].size
	return true
}

func (s DisjointSet) Do(f func(set int)) {
	for i := range s {
		if s[i].parent == i {
			f(i)
		}
	}
}

func Solve(input parsed) (int64, int64) {
	type d struct {
		a, b     int
		distance int64
	}
	distances := make([]d, 0, len(input)*(len(input)-1)/2)

	for i, a := range slices.All(input[:len(input)-1]) {
		for j, b := range slices.All(input[i+1:]) {
			distances = append(distances, d{i, i + 1 + j, (a.X-b.X)*(a.X-b.X) + (a.Y-b.Y)*(a.Y-b.Y) + (a.Z-b.Z)*(a.Z-b.Z)})
		}
	}
	slices.SortFunc(distances, func(a, b d) int { return cmp.Compare(a.distance, b.distance) })

	set := DisjointSet_Create(len(input))

	count := 1000
	if len(input) == 20 {
		//test input
		count = 10
	}

	for i := range count {
		dist := distances[i]
		set.Union(dist.a, dist.b)
	}
	distances = distances[count:]

	//Could optimize this to use a max-K heap, but ehhhh
	sizes := make([]int64, 0)
	set.Do(func(s int) {
		sizes = append(sizes, int64(set[s].size))
	})

	slices.Sort(sizes)
	sizes = sizes[len(sizes)-3:]
	part1_answer := sizes[0] * sizes[1] * sizes[2]

	var part2_answer int64 = 0
	for dist := range slices.Values(distances) {
		if set.Union(dist.a, dist.b) && set[set.FindSet(dist.a)].size == len(input) {
			part2_answer = input[dist.a].X * input[dist.b].X
		}
	}

	return part1_answer, part2_answer
}

func ParseInput(scanner *bufio.Scanner) parsed {
	res := make(parsed, 0)

	for scanner.Scan() {
		line := scanner.Bytes()

		numbers := Map(bytes.Split(line, []byte{','}), func(_ int, num []byte) int64 {
			val, _ := strconv.ParseInt(string(num), 10, 64)
			return val
		})

		res = append(res, coord{numbers[0], numbers[1], numbers[2]})
	}

	return res
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
