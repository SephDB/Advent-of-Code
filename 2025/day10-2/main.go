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

type machine struct {
	target  uint16
	buttons [][]int
	jolts   []int
}

type parsed = []machine

func ParseInput(scanner *bufio.Scanner) parsed {
	res := make(parsed, 0)

	readTarget := func(input []byte) (uint16, []byte) {
		var res uint16 = 0
		start := bytes.IndexByte(input, '[')
		end := bytes.IndexByte(input, ']')
		target := input[start+1 : end]

		for i, b := range slices.All(target) {
			if b == '#' {
				res |= (1 << i)
			}
		}

		return res, input[end+1:]
	}

	readButtons := func(input []byte) ([][]int, []byte) {
		res := make([][]int, 0)

		for start := bytes.IndexByte(input, '('); start != -1; start = bytes.IndexByte(input, '(') {
			end := bytes.IndexByte(input, ')')

			button := make([]int, 0)
			for i := start + 1; i < end; i += 2 {
				button = append(button, int(input[i]-'0'))
			}
			res = append(res, button)

			input = input[end+1:]
		}

		return res, input
	}

	readJoltage := func(input []byte) []int {
		input = input[bytes.IndexByte(input, '{')+1:]
		input = input[:bytes.IndexByte(input, '}')]
		return slices.Collect(Map(bytes.SplitSeq(input, []byte{','}), func(i []byte) int {
			value, _ := strconv.ParseInt(string(i), 10, 32)
			return int(value)
		}))
	}

	for scanner.Scan() {
		line := scanner.Bytes()
		target, line := readTarget(line)
		buttons, line := readButtons(line)

		res = append(res, machine{target, buttons, readJoltage(line)})
	}

	return res
}

type Variable struct {
	min            int
	max            int
	numConstraints int //How many constraints this influences
}

func (v Variable) Size() int {
	return v.Width() + 1
}

func (v Variable) Width() int {
	return v.max - v.min
}

type Constraint struct {
	variables []int
	remaining int
}

func (c Constraint) Clone() Constraint {
	c.variables = slices.Clone(c.variables)
	return c
}

type Problem struct {
	variables   []Variable
	constraints []Constraint
}

func (p Problem) Clone() Problem {
	p.variables = slices.Clone(p.variables)
	constraints := make([]Constraint, len(p.constraints))
	for i := range constraints {
		constraints[i] = p.constraints[i].Clone()
	}
	p.constraints = constraints
	return p
}

func (p Problem) Viable() bool {
	for v := range slices.Values(p.variables) {
		if v.Size() <= 0 {
			return false
		}
	}
	for v := range slices.Values(p.constraints) {
		if v.remaining < 0 || v.GetWiggleRoom(&p) < 0 {
			return false
		}
	}
	return true
}

func (m machine) GetProblem() Problem {
	values := slices.Collect(Map(slices.Values(m.jolts), func(v int) Constraint {
		return Constraint{make([]int, 0, len(m.buttons)), v}
	}))
	variables := make([]Variable, 0, len(m.buttons))

	for i, b := range slices.All(m.buttons) {
		max_val := 10000
		for jolt := range slices.Values(b) {
			max_val = min(max_val, m.jolts[jolt])
			values[jolt].variables = append(values[jolt].variables, i)
		}
		variables = append(variables, Variable{0, max_val, len(b)})
	}

	return Problem{variables, values}
}

func (constraint *Constraint) AvailableRange(p *Problem) (int, int) {
	min_sum := 0
	max_sum := 0
	for v := range slices.Values(constraint.variables) {
		min_sum += p.variables[v].min
		max_sum += p.variables[v].max
	}
	return min_sum, max_sum
}

func (constraint *Constraint) GetWiggleRoom(p *Problem) int {
	_, max := constraint.AvailableRange(p)
	return max - constraint.remaining
}

func (constraint *Constraint) RequiredExtra(p *Problem) int {
	min, _ := constraint.AvailableRange(p)
	return constraint.remaining - min
}

func (p *Problem) PropConstraints() bool {
	changed := false
	for i := range p.constraints {
		constraint := &p.constraints[i]

		//Update affected variables' max
		for v := range slices.Values(constraint.variables) {
			if p.variables[v].max > constraint.remaining {
				p.variables[v].max = constraint.remaining
				changed = true
			}
		}

		//Remove any variables if they have a fixed value, updating the constraint total as you go
		constraint.variables = slices.DeleteFunc(constraint.variables, func(v int) bool {
			variable := &p.variables[v]
			if variable.Size() == 1 {
				changed = true
				constraint.remaining -= variable.max
				return true
			}
			return false
		})

		if len(constraint.variables) == 1 {
			variable := &p.variables[constraint.variables[0]]
			if constraint.remaining >= variable.min && constraint.remaining <= variable.max {
				variable.min = constraint.remaining
				variable.max = constraint.remaining
				constraint.remaining = 0
				constraint.variables = constraint.variables[:0]
				changed = true
			} else {
				//Unsolvable
				variable.max = variable.min - 1
				return true
			}
		} else {
			min_val, max_val := constraint.AvailableRange(p)

			if min_val > constraint.remaining || max_val < constraint.remaining {
				//Error out
				constraint.remaining = -1
				return true
			}

			for v := range slices.Values(constraint.variables) {
				variable := &p.variables[v]
				if min_val+variable.Width() > constraint.remaining {
					variable.max -= min_val + variable.Width() - constraint.remaining
					changed = true
				}
				if max_val-variable.Width() < constraint.remaining {
					variable.min += constraint.remaining - (max_val - variable.Width())
					changed = true
				}
			}
		}
	}

	p.constraints = slices.DeleteFunc(p.constraints, func(c Constraint) bool {
		return c.remaining == 0 && len(c.variables) == 0
	})

	return changed
}

func (p Problem) ChooseBacktrackVar(currentBest int) int {
	//for the most constrained constraint, pick the variable with the least amount of values left that also has the biggest effect on other constraints
	//Idea is that lower variable constraints tend to propagate more values through
	if len(p.constraints) == 0 {
		return -1
	}
	minSlice := slices.MinFunc(p.constraints, func(a, b Constraint) int {
		return cmp.Or(cmp.Compare(len(a.variables), len(b.variables)), cmp.Compare(a.GetWiggleRoom(&p), b.GetWiggleRoom(&p)))
	})

	//Least amount of values left, but most effect on the solution as a whole
	current := Variable{0, currentBest, 0}
	currentIndex := -1
	for i := range slices.Values(minSlice.variables) {
		v := p.variables[i]
		if v.Size() == 1 {
			continue
		}

		if cmp.Or(cmp.Compare(current.numConstraints, v.numConstraints), cmp.Compare(v.Size(), current.Size())) < 0 {
			current = v
			currentIndex = i
		}
	}
	return currentIndex
}

func (p Problem) CurrentMinimum() int {
	res := 0
	for v := range slices.Values(p.variables) {
		res += v.min
	}
	return res
}

func (p Problem) CurrentPressed() int {
	res := 0
	for v := range slices.Values(p.variables) {
		if v.min == v.max {
			res += v.min
		}
	}
	return res
}

func (p Problem) Solved() bool {
	for v := range slices.Values(p.variables) {
		if v.min != v.max {
			return false
		}
	}
	return true
}

func (p Problem) Solve(currentBest int) int {
	for p.PropConstraints() {
		if !p.Viable() {
			//We hit a contradiction after constraint propagation, just return
			return currentBest
		}
	}

	if p.Solved() {
		return min(p.CurrentMinimum(), currentBest)
	}

	requiredPresses := slices.Collect(Map(slices.Values(p.constraints), func(c Constraint) int { return c.RequiredExtra(&p) }))

	if p.CurrentMinimum()+slices.Max(requiredPresses) >= currentBest {
		//Impossible to do better from this point, return instead of backtracking
		return currentBest
	}

	backtracker := p.ChooseBacktrackVar(currentBest)

	for val := range p.variables[backtracker].Size() {
		v := p.variables[backtracker].min + val
		p2 := p.Clone()
		p2.variables[backtracker].max = v
		p2.variables[backtracker].min = v
		currentBest = min(p2.Solve(currentBest), currentBest)
	}

	return currentBest
}

func Part2(input parsed) int64 {
	var result int64 = 0

	for i, m := range slices.All(input) {
		p := m.GetProblem()
		max := 0
		for j := range slices.Values(m.jolts) {
			max += j
		}
		res := p.Solve(max)
		fmt.Println("Solved", i, "Result:", res)
		result += int64(res)
	}

	return result
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)

	input := ParseInput(scanner)

	fmt.Println(Part2(input))

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "error:", err)
		os.Exit(1)
	}
}
