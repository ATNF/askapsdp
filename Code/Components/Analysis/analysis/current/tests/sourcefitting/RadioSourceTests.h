/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2008 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <askap_analysis.h>
#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Component.h>
#include <sourcefitting/FittingParameters.h>

#include <cppunit/extensions/HelperMacros.h>

#include <duchamp/duchamp.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Cubes/cubes.hh>
#include <duchamp/fitsHeader.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Detection/finders.hh>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <Common/ParameterSet.h>

#include <string>
#include <vector>
#include <math.h>

ASKAP_LOGGER(logger, ".radioSourceTest");

namespace askap {

  namespace analysis {

    namespace sourcefitting {
	
      const size_t srcDim=10;
      const size_t srcSize=srcDim*srcDim;
      const size_t arrayDim=10;
      const size_t arraySize=arrayDim*arrayDim;
      const float gaussNorm=10.;
      const float gaussXFWHM=4.;
      const float gaussYFWHM=2.;
      const float gaussX0=5.;
      const float gaussY0=5.;
      const float SIGMAtoFWHM=2. * M_SQRT2 * sqrt(M_LN2);

      class RadioSourceTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(RadioSourceTest);
	CPPUNIT_TEST(findSource);
	CPPUNIT_TEST(sourceBox);
	CPPUNIT_TEST(findGaussSource);
	CPPUNIT_TEST(testShapeGaussSource);
	CPPUNIT_TEST(subthreshold); 
	CPPUNIT_TEST(fitSource);
	CPPUNIT_TEST_SUITE_END();

      private:

	casa::Vector<float>              itsArray;
	casa::Vector<long>               itsDim;
	std::vector<PixelInfo::Object2D> itsObjlist;
	std::vector<SubComponent>        itsSublist;
	RadioSource                      itsSource;
	FittingParameters                itsFitparams;
	duchamp::Section                 itsSection;

	casa::Vector<float>              itsGaussArray;
	std::vector<PixelInfo::Object2D> itsGaussObjlist;
	RadioSource                      itsGaussSource;

      public:

/*****************************************/
	void setUp() {
	  
	  const float src[arraySize]={1.,1.,1.,1.,1.,1.,1.,1.,1.,1.,
				      1.,1.,1.,1.,1.,1.,1.,1.,1.,1.,
				      1.,1.,1.,1.,1.,1.,1.,1.,1.,1.,
				      1.,1.,1.,1.,1.,9.,11.,1.,1.,1.,
				      1.,1.,1.,1.,1.,10.,10.,1.,1.,1.,
				      1.,1.,1.,40.,39.,51.,50.,20.,19.,1.,
				      1.,1.,1.,41.,40.,50.,49.,20.,22.,1.,
				      1.,1.,1.,1.,1.,28.,30.,1.,1.,1.,
				      1.,1.,1.,1.,1.,33.,27.,1.,1.,1.,
				      1.,1.,1.,1.,1.,1.,1.,1.,1.,1.};
	  float gaussSrc[arraySize];
	  for(size_t i=0; i<arraySize; i++) gaussSrc[i]=0.;
	  float gaussXSigma=gaussXFWHM / SIGMAtoFWHM;
	  float gaussYSigma=gaussYFWHM / SIGMAtoFWHM;
	  for(size_t y=0; y<arrayDim; y++){
	    for(size_t x=0; x<arrayDim; x++) {
	      float xterm=(x-gaussX0)/gaussXSigma;
	      float yterm=(y-gaussY0)/gaussYSigma;
	      gaussSrc[x+y*arrayDim] += gaussNorm * exp( -(0.5*xterm*xterm + 0.5*yterm*yterm));
	    }
	  }

	  itsDim = casa::Vector<long>(2);
	  itsDim[0] = itsDim[1] = arrayDim;

	  std::string secstring=duchamp::nullSection(2); 
	  itsSection = duchamp::Section(secstring); 
	  itsSection.parse(itsDim.data(),2);

	  itsFitparams = FittingParameters(LOFAR::ParameterSet());
	  itsFitparams.setFitTypes(std::vector<std::string>(1,"full"));
	  itsFitparams.setMaxNumGauss(1);
	  /* 	  fitparams.setNumSubThresholds(10); */

	  float thresh=5.;
	  itsArray = casa::Vector<float>(casa::IPosition(1,arraySize),src);
	  duchamp::Image *itsImage = new duchamp::Image(itsDim.data());
	  itsImage->saveArray(itsArray.data(),arraySize);
	  itsImage->stats().setThreshold(thresh);
	  itsImage->setMinSize(1);
	  itsImage->pars().setFlagBlankPix(false);
	  itsObjlist = itsImage->findSources2D();
	  CPPUNIT_ASSERT(itsObjlist.size()==1);
	  duchamp::Detection det; 
	  det.addChannel(0,itsObjlist[0]); 
	  det.calcFluxes(itsArray.data(),itsDim.data());
	  itsSource = RadioSource(det); 
	  itsSource.defineBox(itsSection,itsFitparams,2); 
	  itsSource.setDetectionThreshold(thresh);
	  itsSource.setNoiseLevel(1.);
	  itsSource.setHeader(duchamp::FitsHeader());
	  itsSource.setFitParams(itsFitparams);
	  delete itsImage;

	  float gaussThresh = 1.;
	  itsGaussArray = casa::Vector<float>(casa::IPosition(1,arraySize),gaussSrc);
	  itsImage = new duchamp::Image(itsDim.data());
	  itsImage->saveArray(itsGaussArray.data(),arraySize);
	  itsImage->stats().setThreshold(gaussThresh);
	  itsImage->setMinSize(1);
	  itsImage->pars().setFlagBlankPix(false);
	  itsGaussObjlist = itsImage->findSources2D();
	  CPPUNIT_ASSERT(itsGaussObjlist.size()==1);
	  duchamp::Detection detg; 
	  detg.addChannel(0,itsGaussObjlist[0]); 
	  detg.calcFluxes(itsGaussArray.data(),itsDim.data());
	  itsGaussSource = RadioSource(detg); 
	  itsGaussSource.defineBox(itsSection,itsFitparams,2); 
	  itsGaussSource.setDetectionThreshold(gaussThresh);
	  itsGaussSource.setNoiseLevel(1.);
	  delete itsImage;

	}

/*****************************************/
	void tearDown() {
	  itsObjlist.clear();
	  itsGaussObjlist.clear();
	}

/*****************************************/
	void findSource() {
	  CPPUNIT_ASSERT(itsObjlist.size()==1);
	}

/*****************************************/
	void sourceBox() {
	  itsFitparams.setBoxPadSize(0);
	  itsSource.defineBox(itsSection,itsFitparams,2); 

	  CPPUNIT_ASSERT(itsSource.boxXmin()==3);
	  CPPUNIT_ASSERT(itsSource.boxYmin()==3);
	  CPPUNIT_ASSERT(itsSource.boxXmax()==8);
	  CPPUNIT_ASSERT(itsSource.boxXmax()==8);
	  
	}

/*****************************************/
	void findGaussSource() {
	  CPPUNIT_ASSERT(itsGaussObjlist.size()==1);
	}

/*****************************************/
	void testShapeGaussSource() {
	  double maj,min,pa;
	  CPPUNIT_ASSERT(itsGaussObjlist.size() == 1);
	  itsFitparams.setBoxPadSize(0);
	  itsGaussSource.defineBox(itsSection,itsFitparams,2); 
	  float *fluxarray = new float[itsGaussSource.boxSize()];
	  PixelInfo::Object2D spatMap = itsGaussSource.getSpatialMap();
	  for (int i = 0; i < itsGaussSource.boxSize(); i++) fluxarray[i] = 0.;
	  for(size_t x=0;x<arrayDim;x++) {
	    for(size_t y=0;y<arrayDim;y++){ 
	      if (spatMap.isInObject(x, y)) {
		int loc = (x - itsGaussSource.boxXmin()) + itsGaussSource.boxXsize() * (y - itsGaussSource.boxYmin());
		fluxarray[loc] = itsGaussArray[x+y*itsDim[0]];
	      }
	    }
	  }
	  itsGaussSource.getFWHMestimate(fluxarray,pa,maj,min);
	  CPPUNIT_ASSERT(fabs(maj-2)<1.e-6);
	  CPPUNIT_ASSERT(fabs(min-0.5)<1.e-6);
	  CPPUNIT_ASSERT(fabs(pa-M_PI/2)<1.e-6);
	  delete [] fluxarray;
	}

/*****************************************/
	void subthreshold() { 
	  CPPUNIT_ASSERT(itsObjlist.size()==1);
	   
	  casa::Matrix<casa::Double> itsPos;
	  casa::Vector<casa::Double> itsF;
	  itsPos.resize(arraySize, 2);
	  itsF.resize(arraySize);
	  casa::Vector<casa::Double> curpos(2);
	  curpos = 0;
	  for(size_t x=0;x<arrayDim;x++) {
	    for(size_t y=0;y<arrayDim;y++){ 
	      itsF(x+y*arrayDim) = itsArray[x+y*arrayDim];
	      curpos(0) = x;
	      curpos(1) = y;
	      itsPos.row(x+y*arrayDim) = curpos;
	    }
	  }
	  itsSublist=itsSource.getSubComponentList(itsPos,itsF);	
	   
	  /* ASKAPLOG_DEBUG_STR(logger, "Number of subcomponents = " << itsSublist.size()); */
	  /* for(size_t i=0;i<itsSublist.size();i++){ */
	  /*   ASKAPLOG_DEBUG_STR(logger, "Component " << i << ": " << itsSublist[i]); */
	  /* } */
   
	  CPPUNIT_ASSERT(itsSublist.size()==5); 
	} 

/*****************************************/
	void fitSource() {
 	  /* for(size_t y=0; y<arrayDim; y++){ */
	  /*   for(size_t x=0; x<arrayDim; x++) { */
	  /*     std::cerr << std::setw(12) << itsGaussArray[x+y*arrayDim] << " "; */
	  /*   } */
	  /*   std::cerr << '\n'; */
	  /* } */
	  CPPUNIT_ASSERT(itsGaussObjlist.size() == 1);
	  duchamp::FitsHeader head;
	  head.setBminKeyword(1.);
	  itsGaussSource.setHeader(head);
	  itsGaussSource.setFitParams(itsFitparams);
	  
	  itsGaussSource.fitGauss(itsGaussArray.data(),itsDim.data(),itsFitparams);

	  std::vector<casa::Gaussian2D<Double> > fits = itsGaussSource.gaussFitSet();
	  CPPUNIT_ASSERT(fits.size()==1);
	  CPPUNIT_ASSERT(fabs(fits[0].height()-gaussNorm)<1.e-6);
	  CPPUNIT_ASSERT(fabs(fits[0].majorAxis()-gaussXFWHM)<1.e-6);
	  CPPUNIT_ASSERT(fabs(fits[0].minorAxis()-gaussYFWHM)<1.e-6);
	  CPPUNIT_ASSERT(fabs(fits[0].PA()-M_PI/2.)<1.e-6);
	  CPPUNIT_ASSERT(fabs(fits[0].xCenter()-gaussX0)<1.e-6);
	  CPPUNIT_ASSERT(fabs(fits[0].yCenter()-gaussY0)<1.e-6);
	}


      };

    }

  }

}
