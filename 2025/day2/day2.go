package main

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/glenn-brown/golang-pkg-pcre/src/pkg/pcre"
)

func ScanDelimiter(delim byte) func(data []byte, atEOF bool) (advance int, token []byte, err error) {
	return func(data []byte, atEOF bool) (advance int, token []byte, err error) {
		if atEOF && len(data) == 0 {
			return 0, nil, nil
		}
		if i := bytes.IndexByte(data, delim); i >= 0 {
			return i + 1, data[0:i], nil
		}
		if atEOF {
			return len(data), data, nil
		}
		// Request more data.
		return 0, nil, nil
	}
}

var twice = pcre.MustCompile("^(.+)\\1$", 0)
var all_reg = pcre.MustCompile("^(.+)\\1+$", 0)

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Split(ScanDelimiter(','))
	total := 0
	all := 0

	for scanner.Scan() {
		parts := strings.Split(scanner.Text(), "-")
		a, _ := strconv.Atoi(parts[0])
		b, _ := strconv.Atoi(parts[1])

		for n := a; n <= b; n++ {
			str := strconv.Itoa(n)
			if twice.MatcherString(str, 0).Matches() {
				total += n
				all += n
			} else if all_reg.MatcherString(str, 0).Matches() {
				all += n
			}
		}
	}
	fmt.Println(total)
	fmt.Println(all)
}
