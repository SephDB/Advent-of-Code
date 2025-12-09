package main

import (
	"bufio"
	"bytes"
	"cmp"
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

func Compact[E any](input iter.Seq[E], eq func(E, E) bool, accum func(E, E) E) iter.Seq[E] {
	return func(yield func(E) bool) {
		var current E
		isFirst := true
		for v := range input {
			if isFirst {
				current = v
				isFirst = false
				continue
			}
			if eq(current, v) {
				current = accum(current, v)
			} else {
				if !yield(current) {
					return
				}
				current = v
			}
		}
		if !isFirst {
			yield(current)
		}
	}
}

type coord struct {
	X int64
	Y int64
}

type parsed = []coord

func ParseInput(scanner *bufio.Scanner) parsed {
	res := make(parsed, 0)

	for scanner.Scan() {
		split := bytes.Split(scanner.Bytes(), []byte{','})
		X, _ := strconv.ParseInt(string(split[0]), 10, 64)
		Y, _ := strconv.ParseInt(string(split[1]), 10, 64)
		res = append(res, coord{X, Y})
	}

	return res
}

func Part1(input parsed) int64 {
	var result int64 = 0

	for i, c1 := range slices.All(input) {
		for c2 := range slices.Values(input[i+1:]) {
			width := max(c1.X, c2.X) - min(c1.X, c2.X) + 1
			height := max(c1.Y, c2.Y) - min(c1.Y, c2.Y) + 1
			result = max(result, width*height)
		}
	}

	return result
}

type Interval struct {
	Min int64
	Max int64
}

type Scanline []Interval

func Intersects(a, b Interval) bool {
	return a.Max >= b.Min && a.Min <= b.Max
}

func (s *Scanline) Update(edge Interval) {
	index, found := slices.BinarySearchFunc(*s, edge, func(a, b Interval) int {
		if Intersects(a, b) {
			return 0
		}
		return cmp.Compare(a.Min, b.Min)
	})

	if !found {
		*s = slices.Insert(*s, index, edge)
	} else {
		collided := &(*s)[index]
		if *collided == edge {
			//Perfect cancellation, remove element
			*s = slices.Delete(*s, index, index+1)
		} else if collided.Max == edge.Max {
			//Remove from the right
			collided.Max = edge.Min
		} else if collided.Min == edge.Min {
			//Remove from the left
			collided.Min = edge.Max
		} else if collided.Min == edge.Max {
			//Add to the left
			collided.Min = edge.Min
		} else if collided.Max == edge.Min {
			//Add to the right
			collided.Max = edge.Max
		} else {
			//Split in the middle
			old_max := collided.Max
			collided.Max = edge.Min
			*s = slices.Insert(*s, index+1, Interval{edge.Max, old_max})
		}
	}

	*s = slices.Collect(Compact(slices.Values(*s), Intersects, func(a, b Interval) Interval { return Interval{min(a.Min, b.Min), max(a.Max, b.Max)} }))
}

type Edge struct {
	Y    int64
	size Interval
}

func Part2(input parsed) int64 {
	var result int64 = 0

	slices.SortFunc(input, func(a, b coord) int {
		return cmp.Or(cmp.Compare(a.Y, b.Y), cmp.Compare(a.X, b.X))
	})

	edges := Map(slices.Chunk(input, 2), func(a []coord) Edge {
		return Edge{a[0].Y, Interval{a[0].X, a[1].X}}
	})
	//Assuming no overlap in Y coordinates between edges, true for both test and my input

	type RectCandidate struct {
		Start coord
		Range Interval
	}

	rects := make([]RectCandidate, 0)

	interior := make(Scanline, 0)

	for e := range edges {
		interior.Update(e.size)

		points := []coord{{e.size.Min, e.Y}, {e.size.Max, e.Y}}
		for i, r := range slices.All(rects) {
			//check if either point falls in the rect's candidate area
			for p := range slices.Values(points) {
				if p.X >= r.Range.Min && p.X <= r.Range.Max {
					size := (max(r.Start.X, p.X) - min(r.Start.X, p.X) + 1) * (p.Y - r.Start.Y + 1)
					if size > result {
						result = size
					}
				}
			}

			//adapt rect candidate to available interior
			interior_index := slices.IndexFunc(interior, func(i Interval) bool { return r.Start.X >= i.Min && r.Start.X <= i.Max })
			if interior_index == -1 {
				rects[i].Range.Min = rects[i].Range.Max + 1
			} else {
				interval := interior[interior_index]
				rects[i].Range.Min = max(interval.Min, r.Range.Min)
				rects[i].Range.Max = min(interval.Max, r.Range.Max)
			}
		}

		//Remove rect candidates that were found to be not part of the interior anymore
		rects = slices.DeleteFunc(rects, func(a RectCandidate) bool { return a.Range.Min > a.Range.Max })

		//Add two new rectangle candidates(left and right) for
		for point := range slices.Values(points) {
			index := slices.IndexFunc(interior, func(i Interval) bool { return point.X >= i.Min && point.Y <= i.Max })
			if index == -1 {
				continue
			}
			interval := interior[index]
			rects = append(rects, RectCandidate{point, interval})
		}
	}

	return result
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
