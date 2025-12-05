package main

import (
	"bufio"
	"cmp"
	"fmt"
	"os"
	"slices"
	"strconv"
	"strings"
)

// Like CompactFunc, but merges "equal" values using the merge function
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

type Bound struct {
	low  int64
	high int64
}

func main() {
	var p1_answer uint64 = 0
	var p2_answer uint64 = 0

	p := make([]Bound, 0)

	scanner := bufio.NewScanner(os.Stdin)

	for scanner.Scan() {
		line := scanner.Text()

		if line == "" {
			break
		}

		parts := strings.Split(line, "-")
		a, _ := strconv.ParseInt(parts[0], 10, 64)
		b, _ := strconv.ParseInt(parts[1], 10, 64)

		p = append(p, Bound{a, b})
	}

	slices.SortFunc(p, func(a Bound, b Bound) int {
		return cmp.Compare(a.low, b.low)
	})

	p = MergeFunc(p, func(a Bound, b Bound) bool {
		return b.low <= a.high+1
	}, func(a Bound, b Bound) Bound {
		return Bound{a.low, max(a.high, b.high)}
	})

	for scanner.Scan() {
		test, _ := strconv.ParseInt(scanner.Text(), 10, 64)
		for bound := range slices.Values(p) {
			if test >= bound.low && test <= bound.high {
				p1_answer++
				break
			}
		}
	}

	for bound := range slices.Values(p) {
		fmt.Println(bound.low, "-", bound.high)
		p2_answer += uint64(bound.high) - uint64(bound.low) + 1
	}

	fmt.Println(p1_answer)
	fmt.Println(p2_answer)

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
