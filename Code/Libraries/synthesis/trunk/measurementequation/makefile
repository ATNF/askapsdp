TARGET    = tMEquation
SYNLIB	  = ../libsynthesis.a

CASAROOT = $(CONRAD_PROJECT_ROOT)/ThirdPartyLibraries/casacore/tags/casacore-0.1.0/
CASAINC  = -I$(CASAROOT)/include/casacore -I$(CASAROOT)/include/casacore/casacore-0.1.0
CASALIB  = -L$(CASAROOT)/lib -limages -llattices -lcoordinates -lcomponents -lmsvis -lmsfits -lms -lmeasures -lmeasures_f -ltables -lscimath -lscimath_f -lcasa -lfits -lcfitsio -lblas -llapack -lstdc++ -lwcs
WCSROOT  = $(CONRAD_PROJECT_ROOT)/ThirdPartyLibraries//wcslib/tags/wcslib-4.2/
WCSINC   = -I$(WCSROOT)/include/
LINK      = /usr/bin/g++
LFLAGS    = 

INCDIRS   = -I. $(CASAINC) $(WCSINC) -I..
LIBDIRS   = $(SYNLIB) $(CASALIB)

CXX     := /usr/bin/g++ -Wno-deprecated $(INCDIRS)
CXXFLAGS  = -O2 -DAIPS_DARWIN -DAIPS_STDLIB

OBJECTS   = MEImage.o \
			MERegularSolver.o MEImageSolver.o MESimpleSolver.o MEQuality.o \
            MEParams.o MEComponentEquation.o MEDomain.o MEParamsTable.o \
            MEParamsRep.o MENormalEquations.o MEDesignMatrixRep.o
HEADERS   = MEIterative.h \
			MEImage.h  \
			MERegularSolver.h MEImageSolver.h MESimpleSolver.h MEQuality.h \
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
	$(LINK) $(LFLAGS) -o $(TARGET) $(TARGET).o $(LIBDIRS)

$(SYNLIB): $(OBJECTS) $(HEADERS)
	ar r $(SYNLIB) $(OBJECTS) ../dataaccess/*.o

all: tMEquation $(SYNLIB)

clean:
	-rm -f $(OBJECTS) $(SYNLIB) tMEquation tMEquation.o

realclean: clean
	-rm -f $(TARGET) $(SYNLIB)
