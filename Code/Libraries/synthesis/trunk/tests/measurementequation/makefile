tConvolve: tConvolve.cc
	g++ -O2 -funroll-loops tConvolve.cc -o tConvolve

CASAINC=/Users/cor203/Documents/workspace/CONRAD/ThirdPartyLibraries/casacore/tags/casacore-0.1.1/include/casacore
CASALIB=/Users/cor203/Documents/workspace/CONRAD/ThirdPartyLibraries/casacore/tags/casacore-0.1.1/lib

tConvolveCASA: tConvolveCASA.cc
	g++ -c -O2 -DAIPS_STDLIB -I$(CASAINC) tConvolveCASA.cc
	g++ tConvolveCASA.o -L$(CASALIB) -lcasa_scimath -lcasa_casa -o tConvolveCASA

coverage: tConvolve.cc
	g++ -g -fprofile-arcs -ftest-coverage tConvolve.cc -o tConvolve

perf: tConvolve.cc
	g++ -pg tConvolve.cc -o tConvolve
	./tConvolve

debug: tConvolve.cc
	g++ -O2 -g tConvolve.cc -o tConvolve

test: tConvolve
	./tConvolve

testCASA: tConvolveCASA
	./tConvolveCASA

clean:
	rm ./tConvolve