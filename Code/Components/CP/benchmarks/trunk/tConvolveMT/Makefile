CXX=g++
CFLAGS=-O3 -fstrict-aliasing -Wall
LIBS=-lpthread

EXENAME = tConvolveMT
OBJS = tConvolveMT.o Stopwatch.o Benchmark.o

all:		$(EXENAME)

%.o:		%.cc %.h
		$(CXX) $(CFLAGS) -c $<

$(EXENAME):	$(OBJS)
		$(CXX) $(CFLAGS) -o $(EXENAME) $(OBJS) $(LIBS)

clean:
		rm -f $(EXENAME) *.o

