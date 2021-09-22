package bench

import (
	"bytes"
	"io"
	"io/ioutil"
	"regexp"
)

var reHelloWorld = regexp.MustCompile(`Hello, ([^!]+)!`)

func SlowMatchHelloWorld(s string) bool {
	var re = regexp.MustCompile(`Hello, ([^!]+)!`)
	return re.MatchString(s)
}

func FastMatchHelloWorld(s string) bool {
	return reHelloWorld.MatchString(s)
}

func ByValue(buf bytes.Buffer) {
	io.Copy(ioutil.Discard, &buf)
}

func ByReference(buf *bytes.Buffer) {
	io.Copy(ioutil.Discard, buf)
}
