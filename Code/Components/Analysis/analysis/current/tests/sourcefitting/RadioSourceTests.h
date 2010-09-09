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
#include <Common/ParameterSet.h>

#include <string>
#include <vector>
#include <math.h>

ASKAP_LOGGER(logger, ".radioSourceTest");

namespace askap {

  namespace analysis {

    namespace sourcefitting {
	
      const int srcDim=8;
      const int srcSize=srcDim*srcDim;
      const int arrayDim=10;
      const int arraySize=arrayDim*arrayDim;
      const float src[srcSize]={1.,1.,1.,1.,1.,1.,1.,1.,
				1.,1.,1.,9.,11.,1.,1.,1.,
				1.,1.,1.,10.,10.,1.,1.,1.,
				1.,40.,39.,51.,50.,20.,19.,1.,
				1.,41.,40.,50.,49.,20.,22.,1.,
				1.,1.,1.,28.,30.,1.,1.,1.,
				1.,1.,1.,31.,27.,1.,1.,1.,
				1.,1.,1.,1.,1.,1.,1.,1.};
      

      class RadioSourceTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(RadioSourceTest);
	CPPUNIT_TEST(findSource);
	CPPUNIT_TEST(subthreshold); 
	CPPUNIT_TEST_SUITE_END();

      private:

	float                      *itsArray;
	long                       *itsDim;
	std::vector<PixelInfo::Object2D>      itsObjlist;
	std::vector<SubComponent>  itsSublist;
	RadioSource                itsSource;
	FittingParameters          itsFitparams;

      public:

	void setUp() {
	  
	  ASKAPLOG_DEBUG_STR(logger, "****setUp****");
	  
	  itsArray = new float[arraySize];
	  for(int i=0;i<arraySize;i++) itsArray[i] = 0.;
	  

	  for(int x=0;x<srcDim;x++) {
	    for(int y=0;y<srcDim;y++){ 
	      itsArray[(x+1)+(y+1)*arrayDim]=src[x+y*srcDim];
	    }
	  }	      

	  std::cerr << itsArray[45] << "\n";
							
	  itsDim = new long[2];
	  itsDim[0] = itsDim[1] = arrayDim;

	  duchamp::Image *itsImage = new duchamp::Image(itsDim);
	  itsImage->saveArray(itsArray,arraySize);
	  std::cerr << itsImage->getSize() << " " << itsImage->getArray()[45] << "\n";
	  itsImage->pars().setThreshold(5);
	  itsImage->pars().setFlagUserThreshold(true);
	  itsImage->stats().setThreshold(5);
	  itsImage->pars().setMinPix(1);

// 	  itsObjlist = itsImage->findSources2D();
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
	  ASKAPLOG_DEBUG_STR(logger, "****tearDown****");
	  /* 	  delete itsImage; */
	  delete [] itsArray;
	  delete [] itsDim;
	  itsObjlist.clear();
	}

	void findSource() {
	  ASKAPLOG_DEBUG_STR(logger, "Num objects = " << itsObjlist.size());
	  CPPUNIT_ASSERT(itsObjlist.size()==1);
	}

	void subthreshold() { 
	  ASKAPLOG_DEBUG_STR(logger,"Defining detection"); 
	  /* 	   for(size_t i=0;i<100;i++) std::cerr << itsArray[i] << "\n"; */
	  ASKAPLOG_DEBUG_STR(logger, "Num objects = " << itsObjlist.size()); 
	  CPPUNIT_ASSERT(itsObjlist.size()==1);
	  duchamp::Detection det; 
	  det.addChannel(0,itsObjlist[0]); 
	  det.calcFluxes(itsArray,itsDim);
	  ASKAPLOG_DEBUG_STR(logger,"Defining radiosource"); 
	  itsSource = RadioSource(det); 
	  std::string secstring=duchamp::nullSection(2); 
	  duchamp::Section sec(secstring); 
	  sec.parse(itsDim,2);
	  ASKAPLOG_DEBUG_STR(logger,"Defining box"); 
	  itsSource.defineBox(sec,itsFitparams,2); 
	  ASKAPLOG_DEBUG_STR(logger,"Done - size is " << itsSource.boxSize()); 
	  itsSource.setDetectionThreshold(5);
	  itsSource.setNoiseLevel(1.);
	  itsSource.setHeader(duchamp::FitsHeader());
	  itsSource.setFitParams(itsFitparams);
	   
	  ASKAPLOG_DEBUG_STR(logger,"Finding subcomponents"); 
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
	    //ASKAPLOG_DEBUG_STR(logger, "Component " << i << ": " << itsSublist[i]);
	    std::cerr <<  "Component " << i << ": " << itsSublist[i] << "\n";
	  }
   
	  CPPUNIT_ASSERT(itsSublist.size()==5); 
	} 

      };

    }

  }

}
