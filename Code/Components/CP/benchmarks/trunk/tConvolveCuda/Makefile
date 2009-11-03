CXX = g++
CFLAGS = -Wall -O2 -fstrict-aliasing

EXENAME = tConvolveCuda
EXEOBJS = $(EXENAME).o Stopwatch.o
EXELIBS = -lCudaGridder

LIBNAME=libCudaGridder.so
LIBOBJS=CudaGridder.o CudaGridKernel.o 

NVCCFLAGS=-O3

all:		$(EXENAME) $(LIBNAME)

%.o:		%.cc %.h
		$(CXX) $(CFLAGS) -c $<

$(LIBNAME):	CudaGridder.cc CudaGridder.h CudaGridKernel.cu
		nvcc $(NVCCFLAGS) -I/usr/local/cuda/include -c CudaGridKernel.cu
		$(CXX) -Wall -O2 -I/usr/local/cuda/include -c CudaGridder.cc
		nvcc $(NVCCFLAGS) -shared -o $(LIBNAME) $(LIBOBJS)

$(EXENAME):	$(EXEOBJS) $(LIBNAME)
		$(CXX) $(CFLAGS) -L. -o $(EXENAME) $(EXEOBJS) $(EXELIBS)

clean: 
		rm -f $(EXENAME) $(LIBNAME) *.o

