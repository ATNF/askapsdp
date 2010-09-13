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
	
      const int srcDim=10;
      const int srcSize=srcDim*srcDim;
      const int arrayDim=10;
      const int arraySize=arrayDim*arrayDim;

      class RadioSourceTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(RadioSourceTest);
	CPPUNIT_TEST(findSource);
	CPPUNIT_TEST(subthreshold); 
	CPPUNIT_TEST_SUITE_END();

      private:

	casa::Vector<float>              itsArray;
	casa::Vector<long>               itsDim;
	std::vector<PixelInfo::Object2D> itsObjlist;
	std::vector<SubComponent>        itsSublist;
	RadioSource                      itsSource;
	FittingParameters                itsFitparams;

      public:

	void setUp() {
	  
	  const int arrayDim=10;
	  const int arraySize=arrayDim*arrayDim;
	  const float src[arraySize]={1.,1.,1.,1.,1.,1.,1.,1.,1.,1.,
				      1.,1.,1.,1.,1.,1.,1.,1.,1.,1.,
				      1.,1.,1.,1.,1.,1.,1.,1.,1.,1.,
				      1.,1.,1.,1.,1.,9.,11.,1.,1.,1.,
				      1.,1.,1.,1.,1.,10.,10.,1.,1.,1.,
				      1.,1.,1.,40.,39.,51.,50.,20.,19.,1.,
				      1.,1.,1.,41.,40.,50.,49.,20.,22.,1.,
				      1.,1.,1.,1.,1.,28.,30.,1.,1.,1.,
				      1.,1.,1.,1.,1.,31.,27.,1.,1.,1.,
				      1.,1.,1.,1.,1.,1.,1.,1.,1.,1.};
 	  
	  itsArray = casa::Vector<float>(casa::IPosition(1,arraySize),src);

	  itsDim = casa::Vector<long>(2);
	  itsDim[0] = itsDim[1] = arrayDim;

	  duchamp::Image *itsImage = new duchamp::Image(itsDim.data());
	  itsImage->saveArray(itsArray.data(),arraySize);
	  itsImage->pars().setThreshold(5);
	  itsImage->pars().setFlagUserThreshold(true);
	  itsImage->stats().setThreshold(5);
	  itsImage->pars().setMinPix(1);

// 	  itsObjlist = itsImage->findSources2D();
	  std::cerr << itsObjlist.size() << itsImage->isDetection(4,5) << "\n";
	  std::vector<PixelInfo::Object2D> newlist = itsImage->findSources2D();
	  itsObjlist = newlist;
	  std::cerr << newlist.size() << itsObjlist.size() << itsImage->isDetection(4,5) << "\n";
// 	  std::cerr << itsImage->pars() << "\n";
	  ASKAPLOG_DEBUG_STR(logger, "Num objects = " << itsObjlist.size()); 
	  delete itsImage;

	  itsFitparams = FittingParameters(LOFAR::ParameterSet());
	  /* 	  fitparams.setNumSubThresholds(10); */

	}

	void tearDown() {
	  /* 	  delete itsImage; */
	  /* delete [] itsArray; */
	  /* delete [] itsDim; */
	  itsObjlist.clear();
	}

	void findSource() {
	  ASKAPLOG_DEBUG_STR(logger,"FindSource test"); 
	  ASKAPLOG_DEBUG_STR(logger, "Num objects = " << itsObjlist.size());
	  CPPUNIT_ASSERT(itsObjlist.size()==1);
	}

	void subthreshold() { 
	  ASKAPLOG_DEBUG_STR(logger,"SubThreshold test"); 
	  ASKAPLOG_DEBUG_STR(logger, "Num objects = " << itsObjlist.size()); 
	  CPPUNIT_ASSERT(itsObjlist.size()==1);
	  duchamp::Detection det; 
	  det.addChannel(0,itsObjlist[0]); 
	  det.calcFluxes(itsArray.data(),itsDim.data());
	  itsSource = RadioSource(det); 
	  std::string secstring=duchamp::nullSection(2); 
	  duchamp::Section sec(secstring); 
	  sec.parse(itsDim.data(),2);
	  itsSource.defineBox(sec,itsFitparams,2); 
	  itsSource.setDetectionThreshold(5);
	  itsSource.setNoiseLevel(1.);
	  itsSource.setHeader(duchamp::FitsHeader());
	  itsSource.setFitParams(itsFitparams);
	   
	  casa::Matrix<casa::Double> itsPos;
	  casa::Vector<casa::Double> itsF;
	  itsPos.resize(arraySize, 2);
	  itsF.resize(arraySize);
	  casa::Vector<casa::Double> curpos(2);
	  curpos = 0;
	  for(int x=0;x<arrayDim;x++) {
	    for(int y=0;y<arrayDim;y++){ 
	      itsF(x+y*arrayDim) = itsArray[x+y*arrayDim];
	      curpos(0) = x;
	      curpos(1) = y;
	      itsPos.row(x+y*arrayDim) = curpos;
	    }
	  }
	  itsSublist=itsSource.getSubComponentList(itsPos,itsF);	
	   
	  ASKAPLOG_DEBUG_STR(logger, "Number of subcomponents = " << itsSublist.size());
	  for(size_t i=0;i<itsSublist.size();i++){
	    ASKAPLOG_DEBUG_STR(logger, "Component " << i << ": " << itsSublist[i]);
	  }
   
	  CPPUNIT_ASSERT(itsSublist.size()==5); 
	} 

      };

    }

  }

}
