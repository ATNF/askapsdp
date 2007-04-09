TARGET    = tMEquation
SYNLIB	  = ../libsynthesis.a

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
CXXFLAGS  = -g -DAIPS_DARWIN -DAIPS_NO_TEMPLATE_SRC

OBJECTS   = MEImage.o \
			MESolver.o MESimpleSolver.o MEQuality.o \
            MEParams.o MEComponentEquation.o MEDomain.o MEParamsTable.o \
            MEParamsRep.o MENormalEquations.o MEDesignMatrixRep.o MEDesignMatrix.o
HEADERS   = MEIterative.h \
			MEImage.h  \
			MESolver.h MESimpleSolver.h MEQuality.h \
			MEParams.h MEquation.h \
            MEComponentEquation.h MEDomain.h MEParamsTable.h \
            MEParamsRep.h MEParamsRep.tcc MENormalEquations.h \
            MEDesignMatrixRep.h MEDesignMatrixRep.tcc

MEParamsRep.cc: MEParamsRep.tcc MEParamsRep.h
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o MEParamsRep.o MEParamsRep.cc

MEDesignMatrixRep.cc: MEDesignMatrixRep.tcc MEDesignMatrixRep.h
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o MEDesignMatrixRep.o MEDesignMatrixRep.cc

.SUFFIXES: .c .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

$(TARGET):  $(OBJECTS) $(TARGET).cc $(SYNLIB)
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cc
	$(LINK) $(LFLAGS) -o $(TARGET) $(TARGET).o $(SYNLIB) $(LIBDIRS)

$(SYNLIB): $(OBJECTS) $(HEADERS)
	ar r $(SYNLIB) $(OBJECTS) ../dataaccess/*.o

all: tMEquation $(SYNLIB)

clean:
	-rm -f $(OBJECTS) $(SYNLIB) tMEquation tMEquation.o

realclean: clean
	-rm -f $(TARGET) $(SYNLIB)
