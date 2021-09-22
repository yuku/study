package bench

import "testing"

func BenchmarkMatchHelloWorld(b *testing.B) {
	data := map[string]func(string) bool{
		"Slow": SlowMatchHelloWorld,
		"Fast": FastMatchHelloWorld,
	}
	for name, f := range data {
		b.Run(name, func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				f(`Hello, World!`)
			}
		})
	}
}

func BenchmarkSlick(b *testing.B) {
	var max = 255
	data := map[string]func(){
		"Slow": func() {
			var buf []byte
			for j := 0; j < max; j++ {
				buf = append(buf, byte(j))
			}
		},
		"Fast": func() {
			buf := make([]byte, 0, max)
			for j := 0; j < max; j++ {
				buf = append(buf, byte(j))
			}
		},
	}
	for name, f := range data {
		b.Run(name, func(b *testing.B) {
			for i := 0; i < b.N; i++ {
				f()
			}
		})
	}
}
