/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2011 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>
#include <sourcefitting/RadioSource.h>
#include <cppunit/extensions/HelperMacros.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>
#include <Common/KVpair.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Slicer.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <imageaccess/CasaImageAccess.h>
#include <duchamp/Detection/finders.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <string>
#include <math.h>

ASKAP_LOGGER(logger, ".extractionTest");
namespace askap {

  namespace analysis {

    const bool doScale=true;

    class ExtractionTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE(ExtractionTest);
      CPPUNIT_TEST(readParset);
      CPPUNIT_TEST(loadSource);
      CPPUNIT_TEST(extractSpectrum);
      CPPUNIT_TEST_SUITE_END();

    private:
      SpectralBoxExtractor extractor;
      LOFAR::ParameterSet parset; // used for defining the subdef
      std::string tempImage;
      std::string outfile;
      RadioSource object;
	      
      // std::vector<double> beam;

    public:

      void setUp() {
		  
	tempImage="tempImageForExtractionTest";
	outfile="tempOutputFromExtractionTest";
	// beam[0] = 
		  
	// make a synthetic array
	double pixels[81]={16.,16.,16.,16.,16.,16.,16.,16.,16.,
			   16.,12.,12.,12.,12.,12.,12.,12.,16.,
			   16.,12., 8., 8., 8., 8., 8.,12.,16.,
			   16.,12., 8., 4., 4., 4., 8.,12.,16.,
			   16.,12., 8., 4., 1., 4., 8.,12.,16.,
			   16.,12., 8., 4., 4., 4., 8.,12.,16.,
			   16.,12., 8., 8., 8., 8., 8.,12.,16.,
			   16.,12.,12.,12.,12.,12.,12.,12.,16.,
			   16.,16.,16.,16.,16.,16.,16.,16.,16.};

	casa::IPosition shape(3,9,9,10),shapeSml(3,9,9,1);
	casa::Array<Float> array(shape),arrSml(shapeSml);
	for(int y=0;y<9;y++){
	  for(int x=0;x<9;x++){
	    for(int z=0;z<10;z++){
	      casa::IPosition loc(3,x,y,z);
	      array(loc)=float(1./pixels[y*9+x]);
	    }
	    casa::IPosition locSml(3,x,y,0);
	    arrSml(locSml)=float(1./pixels[y*9+x]);
	  }
	}
	accessors::CasaImageAccess ia;
	ia.create(tempImage,shape,casa::CoordinateUtil::defaultCoords3D());
	ia.write(tempImage,array);

	std::vector<bool> mask(81,false); //just the first channel
	for(size_t i=0;i<81;i++) mask[i]=(arrSml.data()[i]>0.5);
	std::vector<PixelInfo::Object2D> objlist=duchamp::lutz_detect(mask,9,9,1);
	CPPUNIT_ASSERT(objlist.size()==1);
	object.addChannel(0,objlist[0]);
	size_t dim[2]; dim[0] = dim[1] = 9;
	object.calcFluxes(arrSml.data(),dim); // should now have the peak position.
	object.setID(1);

	parset.add("spectralCube",tempImage);
	parset.add(LOFAR::KVpair("spectralBoxWidth", 5));
	parset.add(LOFAR::KVpair("scaleSpectraByBeam",doScale));
	parset.add("spectralOutputBase",outfile);

      }

      void readParset() {
	extractor = SpectralBoxExtractor(parset);
	extractor.setBeamScale();
	CPPUNIT_ASSERT(extractor.inputCube() == tempImage);
	CPPUNIT_ASSERT(extractor.outputFile() == outfile);
	CPPUNIT_ASSERT(extractor.boxWidth() == 5);
	CPPUNIT_ASSERT(extractor.doScale() == doScale);
      }

      void loadSource() {
	extractor = SpectralBoxExtractor(parset);
	extractor.setSource(object);
	std::string shouldget=outfile + "_1";
	CPPUNIT_ASSERT(extractor.outputFile() == shouldget);
      }

      void extractSpectrum() {
	extractor = SpectralBoxExtractor(parset);
	extractor.setSource(object);
	for(int width=1;width<=9;width += 2){
	  extractor.setBoxWidth(width);
	  extractor.extract();
	  std::vector<float> asVec;
	  extractor.array().tovector(asVec);
	  for(size_t i=0;i<asVec.size();i++){
/* 	    ASKAPLOG_DEBUG_STR(logger, "Width = "<<width<<", chan="<<i<<", spectrum="<<asVec[i] << ", diff="<<asVec[i]-width); */
	    CPPUNIT_ASSERT(fabs(asVec[i]-width)<1.e-5);
	  }
	  
	}
      }

      void tearDown() {
      }

    };

  }
}
