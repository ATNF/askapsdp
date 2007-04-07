TARGET    = tMEParams
SYNLIB	  = ../../libsynthesis.a

AIPSROOT := $(word 1, $(AIPSPATH))
AIPSARCH := $(word 2, $(AIPSPATH))
AIPSLIB := $(AIPSROOT)/$(AIPSARCH)/lib

CASAINC = -I$(AIPSROOT)/code/include -I$(AIPSROOT)/code/casa
CASALIB = -L$(AIPSLIB) $(AIPSLIB)/version.o -limages -llattices -lcoordinates -lcomponents -lmsvis -lmsfits -lms -lmeasures -lmeasures_f -ltables -lscimath -lscimath_f -lcasa -lfits -lcfitsio -lblas -llapack -lstdc++ -lwcs

CPPUNITINC = -I/usr/local/bin/include
CPPUNITLIB = -L/usr/local/lib -lcppunit
LINK      = /usr/bin/g++
LFLAGS    = 

INCDIRS   = -I. $(CASAINC) $(CPPUNITINC) -I.. -I../..
LIBDIRS   = $(CASALIB) $(CPPUNITLIB)

CXX     := /usr/bin/g++ -Wno-deprecated $(INCDIRS)
CXXFLAGS  = -O2 -DAIPS_DARWIN -DAIPS_NO_TEMPLATE_SRC

.SUFFIXES: .c .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

$(TARGET):  $(TARGET).cc $(SYNLIB)
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cc
	$(LINK) $(LFLAGS) -o $(TARGET) $(TARGET).o $(SYNLIB) $(LIBDIRS)

all: $(TARGET)

clean:
	-rm -f $(TARGET)

realclean: clean
	-rm -f $(TARGET)
