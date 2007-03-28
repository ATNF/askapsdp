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
CXXFLAGS  = -O2 -DAIPS_DARWIN -DAIPS_NO_TEMPLATE_SRC

OBJECTS   = IEqImage.o IEqDataAccessor.o IEqDataSource.o IEqParamBase.o IEqParamsBase.o \
			IEqImageParam.o IEqSolver.o IEqImageSolver.o \
            IEqTableDataAccessor.o IEqComponentEquation.o IEqDomain.o IEqParamsTable.o
HEADERS   = IEqImage.h IEqDataAccessor.h IEqDataSource.h IEqParamBase.h IEqParamBase.tcc \
			IEqParamsBase.tcc IEqParam.h IEqParamsBase.h \
			IEqParams.h IEqSolver.h IEqImageSolver.h IEquation.h \
            IEqTableDataAccessor.h IEqComponentEquation.h IEqImageParam.h IEqImageParams.h IEqDomain.h IEqParamsTable.h

.SUFFIXES: .c .o .cc

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

#$(OBJECTS): $(HEADERS)

$(TARGET):  $(OBJECTS) $(TARGET).cc
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) -o $(TARGET).o $(TARGET).cc
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(TARGET).o $(LIBDIRS)


all: tIEquation

clean:
	-rm -f $(OBJECTS) *.a

realclean: clean
	-rm -f $(TARGET)
