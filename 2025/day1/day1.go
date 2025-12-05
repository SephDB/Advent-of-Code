package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
)

func main() {
	count := 0
	clicks := 0
	current := 50

	scanner := bufio.NewScanner(os.Stdin)

	for scanner.Scan() {

		line := scanner.Text()
		dir := line[0]
		amount, _ := strconv.Atoi(line[1:])

		if dir == 'L' {
			amount *= -1
		}
		current += amount
		if current <= 0 {
			clicks += current / (-100)
			if current != amount {
				clicks++
			}
			current = (current%100 + 100) % 100
		} else {
			clicks += current / 100
			current %= 100
		}
		if current == 0 {
			count++
		}
	}

	fmt.Println(count)
	fmt.Println(clicks)

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
