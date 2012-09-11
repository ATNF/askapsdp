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
#include <extraction/SourceSpectrumExtractor.h>
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

ASKAP_LOGGER(logger, ".sourceSpectrumExtractionTest");
namespace askap {

  namespace analysis {

    const bool doScale=false;

    class SourceSpectrumExtractionTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE(SourceSpectrumExtractionTest);
      CPPUNIT_TEST(readParset);
      CPPUNIT_TEST(loadSource);
      CPPUNIT_TEST(extractSpectrum);
      CPPUNIT_TEST(extractSpectrumPowerlaw);
      CPPUNIT_TEST(extractSpectrumBeam);
      CPPUNIT_TEST_SUITE_END();

    private:
      SourceSpectrumExtractor extractor;
      LOFAR::ParameterSet parset; // used for defining the subdef
      std::string tempImage,tempImageGauss,tempImagePL;
      std::string outfile;
      RadioSource object,gaussobject;
      float alpha;
      // std::vector<double> beam;

    public:

      void setUp() {
		  
	tempImage="tempImageForExtractionTest";
	tempImagePL="tempImagePowerlawForExtractionTest";
	tempImageGauss="tempImageGaussianForExtractionTest";
	outfile="tempOutputFromExtractionTest";
	alpha=0.5;

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
	casa::Array<Float> array(shape),arrSml(shapeSml),arrayPL(shape);
	for(int y=0;y<9;y++){
	  for(int x=0;x<9;x++){
	    casa::IPosition locSml(3,x,y,0);
	    arrSml(locSml)=float(1./pixels[y*9+x]);
	    for(int z=0;z<10;z++){
	      casa::IPosition loc(3,x,y,z);
	      array(loc)=arrSml(locSml);
	      arrayPL(loc)=arrSml(locSml) * pow(double(z+1),alpha);
	    }
	  }
	}
	accessors::CasaImageAccess ia;
	ia.create(tempImage,shape,coo);
	ia.write(tempImage,array);
	ia.setUnits(tempImage,"Jy/beam");
	ia.create(tempImagePL,shape,coo);
	ia.write(tempImagePL,arrayPL);
	ia.setUnits(tempImagePL,"Jy/beam");

	std::vector<bool> mask(81,false); //just the first channel
	for(size_t i=0;i<81;i++) mask[i]=(arrSml.data()[i]>0.5);
	std::vector<PixelInfo::Object2D> objlist=duchamp::lutz_detect(mask,9,9,1);
	CPPUNIT_ASSERT(objlist.size()==1);
	object.addChannel(0,objlist[0]);
	size_t dim[2]; dim[0] = dim[1] = 9;
	object.calcFluxes(arrSml.data(),dim); // should now have the peak position.
	object.setID(1);

	//------------------------------------

	//------------------------------------
	// another synthetic array with a gaussian source at the centre
	float bmaj = 4.;
	float bmin = 2.;
	float sigMajsq = bmaj*bmaj/8./M_LN2;
	float sigMinsq = bmin*bmin/8./M_LN2;
	float bpa = M_PI/4.;
	casa::Array<Float> gaussarray(shape),gaussarrSml(shapeSml);
	for(int y=0;y<9;y++){
	  for(int x=0;x<9;x++){
	    double u = (x-4)*cos(bpa) + (y-4)*sin(bpa);
	    double v = (x-4)*sin(bpa) - (y-4)*cos(bpa);
	    casa::IPosition locSml(3,x,y,0);
	    gaussarrSml(locSml) = exp(-0.5*(u*u/sigMajsq + v*v/sigMinsq));
	    for(int z=0;z<10;z++){
	      casa::IPosition loc(3,x,y,z);
	      gaussarray(loc)=gaussarrSml(locSml);
	    }
	  }
	}
	ia.create(tempImageGauss,shape,coo);
	ia.write(tempImageGauss,gaussarray);
	ia.setBeamInfo(tempImageGauss,bmaj*10./3600.*M_PI/180.,bmin*10./3600.*M_PI/180.,bpa);
	ia.setUnits(tempImageGauss,"Jy/beam");
	    
	mask = std::vector<bool>(81,false); //just the first channel
	for(size_t i=0;i<81;i++) mask[i]=(gaussarrSml.data()[i]>0.9);
	objlist=duchamp::lutz_detect(mask,9,9,1);
	CPPUNIT_ASSERT(objlist.size()==1);
	gaussobject.addChannel(0,objlist[0]);
	gaussobject.calcFluxes(gaussarrSml.data(),dim); // should now have the peak position.
	gaussobject.setID(1);


	parset.add("spectralCube",tempImage);
	parset.add(LOFAR::KVpair("spectralBoxWidth", 5));
	parset.add(LOFAR::KVpair("scaleSpectraByBeam",doScale));
	parset.add("spectralOutputBase",outfile);

      }

      void readParset() {
	extractor = SourceSpectrumExtractor(parset);
	CPPUNIT_ASSERT(extractor.inputCube() == tempImage);
	CPPUNIT_ASSERT(extractor.outputFileBase() == outfile);
	CPPUNIT_ASSERT(extractor.boxWidth() == 5);
	CPPUNIT_ASSERT(extractor.doScale() == doScale);
      }

      void loadSource() {
	extractor = SourceSpectrumExtractor(parset);
	extractor.setSource(object);
	std::string shouldget=outfile + "_1";
	CPPUNIT_ASSERT(extractor.outputFile() == shouldget);
      }

      void extractSpectrum() {
	extractor = SourceSpectrumExtractor(parset);
	extractor.setSource(object);
	for(int width=1;width<=9;width += 2){
	  extractor.setBoxWidth(width);
	  extractor.extract();
	  std::vector<float> asVec;
	  extractor.array().tovector(asVec);
	  for(size_t i=0;i<asVec.size();i++){
/* 	    ASKAPLOG_DEBUG_STR(logger, "A " << extractor.boxWidth() << " " << width << " " << i << " " << asVec[i] << " " << fabs(asVec[i]-width)); */
	    CPPUNIT_ASSERT(fabs(asVec[i]-width)<1.e-5);
	  }
	}
      }

      void extractSpectrumPowerlaw() {
	parset.replace("spectralCube",tempImagePL);
	extractor = SourceSpectrumExtractor(parset);
	extractor.setSource(object);
	for(int width=1;width<=9;width += 2){
	  extractor.setBoxWidth(width);
	  extractor.extract();
	  std::vector<float> asVec;
	  extractor.array().tovector(asVec);
	  for(size_t i=0;i<asVec.size();i++){
/* 	    ASKAPLOG_DEBUG_STR(logger, "B " << extractor.boxWidth() << " " << width << " " << width*pow(double(i+1),alpha) << " " << i << " " << asVec[i] << " " << fabs(asVec[i]-width*pow(double(i+1),alpha))); */
	    CPPUNIT_ASSERT(fabs(asVec[i]-width*pow(double(i+1),alpha))<1.e-4);
	  }
	}
      }

      void extractSpectrumBeam() {
	parset.replace("spectralCube",tempImageGauss);
	extractor = SourceSpectrumExtractor(parset);
	extractor.setSource(gaussobject);
	extractor.setFlagDoScale(true);
	for(int width=1;width<=9;width += 2){
	  extractor.setBoxWidth(width);
	  extractor.extract();
	  std::vector<float> asVec;
	  extractor.array().tovector(asVec);
	  for(size_t i=0;i<asVec.size();i++){
/* 	    ASKAPLOG_DEBUG_STR(logger, "C " << extractor.boxWidth() << " " << width << " " << i << " " << asVec[i] << " " << fabs(asVec[i]-1)); */
	    CPPUNIT_ASSERT(fabs(asVec[i]-1.)<1.e-5);
	  }
	}
      }

      void tearDown() {
	std::stringstream ss;
	ss << "rm -rf " << tempImage;
	system(ss.str().c_str());
	ss.str();
	ss << "rm -rf " << tempImagePL;
	system(ss.str().c_str());
	ss.str();
	ss << "rm -rf " << tempImageGauss;
	system(ss.str().c_str());
      }

    };

  }
}
