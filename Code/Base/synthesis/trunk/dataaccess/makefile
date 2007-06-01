TARGET    = tDataAccess

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
CXXFLAGS  = -O2 -DAIPS_DARWIN -DAIPS_NO_TEMPLATE_SRC

OBJECTS   = IConstDataAccessor.o DataIteratorStub.o \
            IDataIterator.o IDataConverter.o IConstDataIterator.o \
	    IConstDataSource.o IDataSource.o DataAccessorStub.o
HEADERS   = IDataAccessor.h IDataSource.h  IConstDataAccessor.h \
            IConstDataIterator.h IDataSelector.h IDataConverter.h \
	    IDataIterator.h IConstDataSource.h IFlagDataAccessor.h \
	    SharedIter.h DataAdapter.h DataIteratorStub.h

.SUFFIXES: .c .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

#$(OBJECTS): $(HEADERS)

all: $(OBJECTS)

$(TARGET):  $(OBJECTS) $(HEADERS) $(TARGET).cc
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cc
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(TARGET).o $(LIBDIRS)


clean:
	-rm -f $(OBJECTS) *.a

realclean: clean
	-rm -f $(TARGET)
