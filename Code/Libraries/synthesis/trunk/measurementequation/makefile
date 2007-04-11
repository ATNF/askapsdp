TARGET    = tMEquation
SYNLIB	  = ../libsynthesis.a

CASAROOT = $(CONRAD_PROJECT_ROOT)/ThirdPartyLibraries/casacore/tags/casacore-0.1.0/
CASAINC  = -I$(CASAROOT)/include/casacore -I$(CASAROOT)/include/casacore/casacore-0.1.0
CASALIB  = -L$(CASAROOT)/lib -lcasa_images -lcasa_mirlib -lcasa_lattices -lcasa_coordinates \
 -lcasa_components -lcasa_msvis -lcasa_msfits -lcasa_ms -lcasa_measures -lcasa_measures_f \
 -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_casa 
WCSROOT  = $(CONRAD_PROJECT_ROOT)/ThirdPartyLibraries//wcslib/tags/wcslib-4.2
WCSLIB   = -L$(WCSROOT)/lib -lwcs
WCSINC   = -I$(WCSROOT)/include/
CFITSROOT = $(CONRAD_PROJECT_ROOT)/ThirdPartyLibraries/cfitsio/tags/cfitsio-3.0.2
CFITSLIB = -L$(CFITSROOT)/lib -lcfitsio
CFITSINC = -I$(CFITSROOT)/include/
LINK      = /usr/bin/g++
LFLAGS    = 

INCDIRS   = -I. $(CASAINC) $(WCSINC) $(CFITSINC) -I..
LIBDIRS   = $(SYNLIB) $(CASALIB) $(WCSLIB) $(CFITSLIB) -lblas -llapack -lstdc++

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

MEParamsRep.o: MEParamsRep.tcc MEParamsRep.h
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o MEParamsRep.o MEParamsRep.cc

MEDesignMatrixRep.o: MEDesignMatrixRep.tcc MEDesignMatrixRep.h
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
