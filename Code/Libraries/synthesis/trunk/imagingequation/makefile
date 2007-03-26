TARGET    = tIEquation

AIPSROOT := $(word 1, $(AIPSPATH))
AIPSARCH := $(word 2, $(AIPSPATH))
AIPSLIB := $(AIPSROOT)/$(AIPSARCH)/lib

CASAINC = -I$(AIPSROOT)/code/include -I$(AIPSROOT)/code/casa
CASALIB = -L$(AIPSLIB) $(AIPSLIB)/version.o -limages -llattices -lcoordinates -lcomponents -lmsvis -lmsfits -lms -lmeasures -lmeasures_f -ltables -lscimath -lscimath_f -lcasa -lfits -lcfitsio -lblas -llapack -lstdc++ -lwcs

LINK      = /usr/bin/g++
LFLAGS    = 

INCDIRS   = -I. $(CASAINC)
LIBDIRS   = $(CASALIB)

CXX     := /usr/bin/g++ -Wno-deprecated $(INCDIRS)
CXXFLAGS  = -ansi -O2 -DAIPS_DARWIN -DAIPS_NO_TEMPLATE_SRC

OBJECTS   = IEqDataAccessor.o IEqDataSource.o IEqParam.o IEqParams.o IEqSolver.o \
            IEqTableDataAccessor.o IEqComponentEquation.o IEqImageParam.o IEqImageParams.o
HEADERS   = IEqDataAccessor.h IEqDataSource.h IEqParam.h IEqParams.h IEqSolver.h IEquation.h \
            IEqTableDataAccessor.h IEqComponentEquation.h IEqImageParam.h IEqImageParams.h

.SUFFIXES: .c .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

$(OBJECTS): $(HEADERS)

$(TARGET):  $(OBJECTS) $(TARGET).cc
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cc
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(TARGET).o $(LIBDIRS)


all: tIEquation

clean:
	-rm -f $(OBJECTS) *.a

realclean: clean
	-rm -f $(TARGET)
