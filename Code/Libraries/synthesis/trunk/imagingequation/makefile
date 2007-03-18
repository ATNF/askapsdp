TARGET    = tIEquation


AIPSROOT=/opt/aips++
AIPSLIB=/opt/aips++/darwin/lib

CASAINC = -I$(AIPSROOT)/code/include -I$(AIPSROOT)/code/casa

CASALIB = -L$(AIPSLIB) $(AIPSLIB)/version.o -lmsvis -lms -ltables -lmeasures -lmeasures_f -lscimath -lscimath_f -lcasa -lblas -llapack -lstdc++

LINK      = /usr/bin/g++
LFLAGS    = 

INCDIRS   = -I. $(CASAINC)
LIBDIRS   = $(CASALIB)

CXX     := /usr/bin/g++ -Wno-deprecated $(INCDIRS)
CXXFLAGS  = -ansi -O2 -DAIPS_DARWIN -DAIPS_NO_TEMPLATE_SRC

OBJECTS   = IEqDataAccessor.o IEqDataSource.o IEqParam.o IEqParams.o IEqSolver.o
HEADERS   = IEqDataAccessor.h IEqDataSource.h IEqParam.h IEqParams.h IEqSolver.h IEquation.h

.SUFFIXES: .c .o .cpp .cc .cxx

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

$(OBJECTS): $(HEADERS)

$(TARGET):  $(OBJECTS) $(TARGET).cpp
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cpp
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(TARGET).o $(LIBDIRS)


all: tIEquation

clean:
	-rm -f $(OBJECTS) *.a

realclean: clean
	-rm -f $(TARGET)
