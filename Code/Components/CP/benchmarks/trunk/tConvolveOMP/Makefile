CXX=g++
CFLAGS=-O2 -fstrict-aliasing -fopenmp

all:		tConvolveOMP

Stopwatch.o:	Stopwatch.cc Stopwatch.h
		$(CXX) $(CFLAGS) -c Stopwatch.cc

tConvolveOMP:	tConvolveOMP.cc Stopwatch.o
		$(CXX) $(CFLAGS) -c tConvolveOMP.cc
		$(CXX) $(CFLAGS) -o tConvolveOMP tConvolveOMP.o Stopwatch.o

clean:
		rm -f *.o tConvolveOMP
