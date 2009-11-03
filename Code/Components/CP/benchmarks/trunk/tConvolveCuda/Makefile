CXX = g++
CFLAGS = -Wall -O2 -fstrict-aliasing -fPIC

EXENAME = tConvolveCuda
EXEOBJS = $(EXENAME).o Stopwatch.o
EXELIBS = -lCudaGridder

LIBNAME=libCudaGridder.so
LIBOBJS=CudaGridder.o CudaGridKernel.o 

NVCCFLAGS=-O3 --compiler-options -fPIC

all:		$(EXENAME) $(LIBNAME)

%.o:		%.cc %.h
		$(CXX) $(CFLAGS) -c $<

$(LIBNAME):	CudaGridder.cc CudaGridder.h CudaGridKernel.cu
		nvcc $(NVCCFLAGS) -I/usr/local/cuda/include -c CudaGridKernel.cu
		$(CXX) $(CFLAGS) -I/usr/local/cuda/include -c CudaGridder.cc
		nvcc $(NVCCFLAGS) -shared -o $(LIBNAME) $(LIBOBJS)

$(EXENAME):	$(EXEOBJS) $(LIBNAME)
		$(CXX) $(CFLAGS) -L. -L/usr/local/cuda -o $(EXENAME) $(EXEOBJS) $(EXELIBS)

clean: 
		rm -f $(EXENAME) $(LIBNAME) *.o

