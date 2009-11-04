CXX=g++
CFLAGS=-O3 -Wall
LIBS=-lpthread

EXENAME = tConvolveMT
OBJS = tConvolveMT.o Stopwatch.o

all:		$(EXENAME)

%.o:		%.cc %.h
		$(CXX) $(CFLAGS) -c $<

$(EXENAME):	$(OBJS)
		$(CXX) $(CFLAGS) -o $(EXENAME) $(OBJS) $(LIBS)

clean:
		rm -f $(EXENAME) *.o

