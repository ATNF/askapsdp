ifdef MAKE_CRAY
	CC       = cc
	CCFLAGS  = -fastsse
	CXX      = CC
	CXXFLAGS = -fastsse -DMPICH_IGNORE_CXX_SEEK -DHAVE_MPI
	#MFLAGS=-Wl,-Map,map.$@
	#EFLAG=-E
	LFLAGS   = -llustre -lm -Mlarge_arrays
else
	CC       = /usr/bin/gcc
	CCFLAGS  =
	CXX      = /usr/bin/g++
	CXXFLAGS = -Wno-deprecated -O2
	LFLAGS   =
endif

INCDIRS = -I.
LIBDIRS =

all: ParallelIO parallel_io
	mv $^ ../bin

ParallelIO.o: ParallelIO.cc
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c $<

parallel_io.o: parallel_io.c
	$(CC) $(CCFLAGS) $(INCDIRS) -c $<

ParallelIO: ParallelIO.o
	$(CXX) $(LFLAGS) $(LIBDIRS) -o $@ $^

parallel_io: parallel_io.o
	$(CC) $(LFLAGS) $(LIBDIRS) -o $@ $^

clean: 
	rm -f *~ ../bin/* *.o *.a a.out core parallel_io ParallelIO
