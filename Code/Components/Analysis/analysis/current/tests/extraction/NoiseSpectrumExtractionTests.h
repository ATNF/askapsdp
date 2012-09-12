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
#include <extraction/NoiseSpectrumExtractor.h>
#include <sourcefitting/RadioSource.h>
#include <cppunit/extensions/HelperMacros.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>
#include <Common/KVpair.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <imageaccess/CasaImageAccess.h>
#include <duchamp/Detection/finders.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <string>
#include <math.h>

//ASKAP_LOGGER(logger, ".noiseSpectrumExtractionTest");
namespace askap {

  namespace analysis {


    class NoiseSpectrumExtractionTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE(NoiseSpectrumExtractionTest);
      CPPUNIT_TEST(readParset);
      CPPUNIT_TEST(loadSource);
      CPPUNIT_TEST(extractSpectrum);
      CPPUNIT_TEST_SUITE_END();

    private:
      NoiseSpectrumExtractor extractor;
      LOFAR::ParameterSet parset; // used for defining the subdef
      std::string tempImage;
      std::string outfile;
      RadioSource object;
      float area,bmaj,bmin,bpa;

    public:

      void setUp() {

	tempImage="tempImageForNoiseExtractionTest";
	area=50.;
	//-----------------------------------
	// Make the coordinate system for the images
	
	Matrix<Double> xform(2,2); xform=0.; xform.diagonal()=1.;
	casa::DirectionCoordinate dircoo(MDirection::J2000,Projection(Projection::SIN),
					 casa::Quantum<Double>(187.5,"deg"),casa::Quantum<Double>(-45.,"deg"),
					 casa::Quantum<Double>(10./3600.,"deg"),casa::Quantum<Double>(10./3600.,"deg"),
					 xform,5,5);
	casa::SpectralCoordinate spcoo(MFrequency::TOPO, 1.4e9, 1.e6, 0, 1420405751.786);
	casa::CoordinateSystem coo=casa::CoordinateUtil::defaultCoords3D();
	coo.replaceCoordinate(dircoo,coo.findCoordinate(casa::Coordinate::DIRECTION));
	coo.replaceCoordinate(spcoo,coo.findCoordinate(casa::Coordinate::SPECTRAL));

	//-----------------------------------
	// make a synthetic array where the box sum of a given width will be equal to the width
	float pixels[81]={-16.,-16.,-16.,-16.,-16.,-16.,-16.,-16.,-16.,
			  -16.,-12.,-12.,-12.,-12.,-12.,-12.,-12.,-16.,
			  -16.,-12., -8., -8., -8., -8., -8.,-12.,-16.,
			  -16.,-12., -8., -4., -4., -4., -8.,-12.,-16.,
			  -16.,-12., -8., -4., -1., -4., -8.,-12.,-16.,
			  -16.,-12., -8., -4., -4., -4., -8.,-12.,-16.,
			  -16.,-12., -8., -8., -8., -8., -8.,-12.,-16.,
			  -16.,-12.,-12.,-12.,-12.,-12.,-12.,-12.,-16.,
			  -16.,-16.,-16.,-16.,-16.,-16.,-16.,-16.,-16.};
	
	casa::IPosition shape(3,9,9,10),shapeSml(3,9,9,1);
	casa::Array<Float> array(shape),arrSml(shapeSml);
	for(int y=0;y<9;y++){
	  for(int x=0;x<9;x++){
	    casa::IPosition locSml(3,x,y,0);
	    arrSml(locSml)=pixels[y*9+x];
	    for(int z=0;z<10;z++){
	      casa::IPosition loc(3,x,y,z);
	      array(loc)=arrSml(locSml);
	    }
	  }
	}
	bmaj = 4.;
	bmin = 2.;
	bpa = M_PI/4.;
	accessors::CasaImageAccess ia;
	ia.create(tempImage,shape,coo);
	ia.write(tempImage,array);
	ia.setBeamInfo(tempImage,bmaj*10./3600.*M_PI/180.,bmin*10./3600.*M_PI/180.,bpa);

	std::vector<bool> mask(81,false); //just the first channel
	for(size_t i=0;i<81;i++) mask[i]=(arrSml.data()[i]>-2.);
	std::vector<PixelInfo::Object2D> objlist=duchamp::lutz_detect(mask,9,9,1);
	CPPUNIT_ASSERT(objlist.size()==1);
	object.addChannel(0,objlist[0]);
	size_t dim[2]; dim[0] = dim[1] = 9;
	object.calcFluxes(arrSml.data(),dim); // should now have the peak position.
	object.setID(1);

	parset.add("spectralCube",tempImage);
	parset.add(LOFAR::KVpair("noiseArea", area));
	parset.add("spectralOutputBase",outfile);

      }

      void readParset(){
	extractor = NoiseSpectrumExtractor(parset);
	CPPUNIT_ASSERT(extractor.inputCube() == tempImage);
	CPPUNIT_ASSERT(extractor.outputFileBase() == outfile);
	CPPUNIT_ASSERT(fabs(extractor.boxArea()-50)<1.e-8);
	CPPUNIT_ASSERT(extractor.boxWidth()==int(ceil(sqrt(50*bmaj*bmin*M_PI))));
      }

      void loadSource() {
	extractor = NoiseSpectrumExtractor(parset);
	extractor.setSource(&object);
	std::string shouldget=outfile + "_1";
	CPPUNIT_ASSERT(extractor.outputFile() == shouldget);
      }

      void extractSpectrum() {
	extractor = NoiseSpectrumExtractor(parset);
	extractor.setSource(&object);
	float madfm[5]={0,0,0,4,4};
	for(int width=1;width<=9;width += 2){
	  float val=madfm[(width-1)/2]/Statistics::correctionFactor;
	  extractor.setBoxWidth(width);
	  extractor.extract();
	  std::vector<float> asVec;
	  extractor.array().tovector(asVec);
	  for(size_t i=0;i<asVec.size();i++){
/* 	    ASKAPLOG_DEBUG_STR(logger, extractor.boxWidth() << " " << width << " " << i << " " << asVec[i] << " " << val << " " << fabs(asVec[i]-val)); */
	    CPPUNIT_ASSERT(fabs(asVec[i]-val)<1.e-7);
	  }
	}
      }

      void tearDown() {
	std::stringstream ss;
	ss << "rm -rf " << tempImage;
	system(ss.str().c_str());
      }

    };

  }
}
