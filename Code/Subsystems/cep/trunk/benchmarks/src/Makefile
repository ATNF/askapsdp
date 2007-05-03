# Default variables
CC       = cc
CCFLAGS  =
CXX      = c++
CXXFLAGS = -Wno-deprecated -O2
LFLAGS   =
INCDIRS  = -I.
LIBDIRS  =

# Machine dependant variables
ifeq ($(HOSTTYPE),x86_64-linux)
	# Cray XT3
	CC       = cc
	CCFLAGS  = -fastsse
	CXX      = CC
	CXXFLAGS = -fastsse -DMPICH_IGNORE_CXX_SEEK -DHAVE_MPI
	LFLAGS   = -llustre -lm -Mlarge_arrays
endif

ifeq ($(HOSTTYPE),i686)
	# IBM PC Blades using MPI wrappers and Intel compilers
	CC       = mpicc -cc=icc
	CCFLAGS  = -fast
	CXX      = mpicxx -cxx=icpc
	CXXFLAGS = -fast -DMPICH_IGNORE_CXX_SEEK -DHAVE_MPI
	LFLAGS   =
endif

all: ParallelIO parallel_io
	mv $^ ../bin

ParallelIO: ParallelIO.o
	$(CXX) $(LFLAGS) $(LIBDIRS) -o $@ $^

ParallelIO.o: ParallelIO.cc
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c $<

parallel_io: parallel_io.o
	$(CC) $(LFLAGS) $(LIBDIRS) -o $@ $^

parallel_io.o: parallel_io.c
	$(CC) $(CCFLAGS) $(INCDIRS) -c $<

clean: 
	rm -rf *~ ../bin/* *.o *.a a.out core parallel_io ParallelIO
