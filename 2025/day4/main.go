package main

import (
	"bufio"
	"fmt"
	"os"
)

func Map[S ~[]E, E, R any](slice S, mapper func(int, E) R) []R {
	mappedSlice := make([]R, len(slice))
	for i, v := range slice {
		mappedSlice[i] = mapper(i, v)
	}
	return mappedSlice
}

func Pop[E any](slice *[]E) E {
	f := len(*slice)
	rv := (*slice)[f-1]
	*slice = (*slice)[:f-1]
	return rv
}

type grid [][]byte

func (p grid) Value(x int, y int) byte {
	if y < 0 || y >= len(p) {
		return 0
	}
	line := p[y]
	if x < 0 || x >= len(line) {
		return 0
	}
	return line[x]
}

type coord struct {
	x int
	y int
}

func main() {
	var p1_answer uint64 = 0
	var p2_answer uint64 = 0

	p := make(grid, 0)

	scanner := bufio.NewScanner(os.Stdin)

	for scanner.Scan() {
		line := scanner.Bytes()

		p = append(p, Map(line, func(i int, c byte) byte {
			if c == '@' {
				return 1
			}
			return 0
		}))
	}

	//Precalculate neighbour counts once
	for y, line := range p {
		for x, cell := range line {
			if cell > 0 {
				for dy := -1; dy <= 1; dy++ {
					for dx := -1; dx <= 1; dx++ {
						if (dx != 0 || dy != 0) && p.Value(x+dx, y+dy) > 0 {
							p[y+dy][x+dx]++
						}
					}
				}
			}
		}
	}

	removable := make([]coord, 0)

	for y, line := range p {
		for x, cell := range line {
			if cell != 0 && cell < 5 {
				p1_answer++
				removable = append(removable, coord{x, y})
			}
		}
	}

	for len(removable) > 0 {
		p2_answer++
		c := Pop(&removable)
		p[c.y][c.x] = 0
		for dy := -1; dy <= 1; dy++ {
			for dx := -1; dx <= 1; dx++ {
				if p.Value(c.x+dx, c.y+dy) > 0 {
					val := &p[c.y+dy][c.x+dx]
					*val--
					if *val == 4 {
						removable = append(removable, coord{c.x + dx, c.y + dy})
					}
				}
			}
		}
	}

	fmt.Println(p1_answer)
	fmt.Println(p2_answer)

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
