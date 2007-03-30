TARGET    = tMEuation

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

OBJECTS   = MEImage.o MEDataAccessor.o MEDataSource.o MEParamBase.o MEParamsBase.o \
			MEImageParam.o MESolver.o MEImageSolver.o \
            METableDataAccessor.o MEComponentEquation.o MEDomain.o MEParamsTable.o
HEADERS   = MEImage.h MEDataAccessor.h MEDataSource.h MEParamBase.h MEParamBase.tcc \
			MEParamsBase.tcc MEParam.h MEParamsBase.h \
			MEParams.h MESolver.h MEImageSolver.h MEuation.h \
            METableDataAccessor.h MEComponentEquation.h MEImageParam.h MEImageParams.h MEDomain.h MEParamsTable.h

.SUFFIXES: .c .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

#$(OBJECTS): $(HEADERS)

$(TARGET):  $(OBJECTS) $(TARGET).cc
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cc
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(TARGET).o $(LIBDIRS)


all: tMEuation

clean:
	-rm -f $(OBJECTS) *.a

realclean: clean
	-rm -f $(TARGET)
