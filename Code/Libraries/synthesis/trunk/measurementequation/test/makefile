TARGET    = tMEDomain
SYNLIB	  = ../../libsynthesis.a

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

CPPUNITINC = -I/usr/local/bin/include
CPPUNITLIB = -L/usr/local/lib -lcppunit
LINK      = /usr/bin/g++
LFLAGS    = 

INCDIRS   = -I. $(CASAINC) $(WCSINC) $(CFITSINC) $(CPPUNITINC) -I.. -I../..
LIBDIRS   = $(SYNLIB) $(CASALIB) $(WCSLIB) $(CFITSLIB) $(CPPUNITLIB) -lblas -llapack -lstdc++

CXX     := /usr/bin/g++ -Wno-deprecated $(INCDIRS)
CXXFLAGS  = -g -DAIPS_DARWIN -DAIPS_NO_TEMPLATE_SRC

.SUFFIXES: .c .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

$(TARGET):  $(TARGET).cc $(SYNLIB)
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cc
	$(LINK) $(LFLAGS) -o $(TARGET) $(TARGET).o $(SYNLIB) $(LIBDIRS)

$(SYNLIB):
	(cd ..;make all)
	
all: $(TARGET) $(SYNLIB)

clean:
	-rm -f tMEParams tMEDomain
	