tConvolve: tConvolve.cc
	g++ -O2 -ftree-vectorize tConvolve.cc -o tConvolve

coverage: tConvolve.cc
	g++ -g -fprofile-arcs -ftest-coverage tConvolve.cc -o tConvolve
	
perf: tConvolve.cc
	g++ -pg tConvolve.cc -o tConvolve
	./tConvolve

debug: tConvolve.cc
	g++ -O2 -g tConvolve.cc -o tConvolve

test: tConvolve
	./tConvolve

clean:
	rm ./tConvolve