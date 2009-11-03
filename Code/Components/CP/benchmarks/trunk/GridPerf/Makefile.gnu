CXX=g++
CFLAGS=-O3 -Wall
LIBS=-lpthread

EXENAME = GridPerf
OBJS = GridPerf.o Stopwatch.o

all:		$(EXENAME)

%.o:		%.cc %.h
			$(CXX) $(CFLAGS) -c $<

$(EXENAME):	$(OBJS)
			$(CXX) $(CFLAGS) -o $(EXENAME) $(OBJS) $(LIBS)

clean:
		    rm -f $(EXENAME) *.o

