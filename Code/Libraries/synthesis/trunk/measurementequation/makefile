TARGET    = tMEquation

AIPSROOT := $(word 1, $(AIPSPATH))
AIPSARCH := $(word 2, $(AIPSPATH))
AIPSLIB := $(AIPSROOT)/$(AIPSARCH)/lib

CASAINC = -I$(AIPSROOT)/code/include -I$(AIPSROOT)/code/casa
CASALIB = -L$(AIPSLIB) $(AIPSLIB)/version.o -limages -llattices -lcoordinates -lcomponents -lmsvis -lmsfits -lms -lmeasures -lmeasures_f -ltables -lscimath -lscimath_f -lcasa -lfits -lcfitsio -lblas -llapack -lstdc++ -lwcs

LINK      = /usr/bin/g++
LFLAGS    = 

INCDIRS   = -I. $(CASAINC) -I..
LIBDIRS   = $(CASALIB)

CXX     := /usr/bin/g++ -Wno-deprecated $(INCDIRS)
CXXFLAGS  = -O2 -DAIPS_DARWIN -DAIPS_NO_TEMPLATE_SRC

OBJECTS   = MEImage.o \
			MESolver.o MESimpleSolver.o MEQuality.o \
            MEComponentEquation.o MEDomain.o MEParamsTable.o \
            MEParamsRep.o
HEADERS   = Iterative.h \
			MEImage.h  \
			MESolver.h MESimpleSolver.h MEQuality.h \
			MEParams.h MEquation.h \
            MEComponentEquation.h MEDomain.h MEParamsTable.h \
            MEParamsRep.h MEParamsRep.tcc

.SUFFIXES: .c .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

#$(OBJECTS): $(HEADERS)

$(TARGET):  $(OBJECTS) $(TARGET).cc
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cc
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(TARGET).o $(LIBDIRS)


all: tMEquation

clean:
	-rm -f $(OBJECTS) *.a tMEquation tMEquation.o

realclean: clean
	-rm -f $(TARGET)
