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

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/Component.h>
#include <sourcefitting/FittingParameters.h>
#include <cppunit/extensions/HelperMacros.h>
#include <duchamp/duchamp.hh>
#include <duchamp/Detection/detection.hh>
#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Array.h>

#include <string>
#include <vector>
#include <math.h>

ASKAP_LOGGER(logger, ".radioSourceTest");

namespace askap {

  namespace analysis {

    namespace sourcefitting {
				
      class RadioSourceTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(RadioSourceTest);
	CPPUNIT_TEST(findSource);
	/* CPPUNIT_TEST(subthreshold); */
	CPPUNIT_TEST_SUITE_END();

      private:

	float *array;
	long *dim;
	casa::Matrix<casa::Double> itsPos;
	casa::Vector<casa::Double> itsF;
	duchamp::Image *image;
	duchamp::Section itsSection;
	std::vector<Object2D> objlist;
	std::vector<SubComponent> sublist;
	RadioSource itsSource;
	FittingParameters fitparams;

      public:

	void setUp() {

	  ASKAPLOG_DEBUG_STR(logger,"Starting the setup");
	  const float src[64]={1.,1.,1.,1.,1.,1.,1.,1.,
			       1.,1.,1.,9.,10.,1.,1.,1.,
			       1.,1.,1.,11.,10.,1.,1.,1.,
			       1.,40.,39.,51.,50.,20.,19.,1.,
			       1.,41.,40.,50.,49.,20.,21.,1.,
			       1.,1.,1.,31.,30.,1.,1.,1.,
			       1.,1.,1.,28.,27.,1.,1.,1.,
			       1.,1.,1.,1.,1.,1.,1.,1.};

	  ASKAPLOG_DEBUG_STR(logger,"Defining the arrays");
	  array = new float[100];
	  itsPos.resize(100, 2);
	  itsF.resize(100);
	  for(int i=0;i<100;i++) array[i] = 0.;
	  casa::Vector<casa::Double> curpos(2);
	  curpos = 0;
	  for(int x=0;x<8;x++) {
	    for(int y=0;y<8;y++){ 
	      array[(x+1)+(y+1)*10]=src[x+y*8];
	    }
	  }	      
	  for(int x=0;x<10;x++) {
	    for(int y=0;y<10;y++){ 
	      itsF(x+y*10) = array[x+y*10];
	      curpos(0) = x;
	      curpos(1) = y;
	      itsPos.row(x+y*10) = curpos;
	    }
	  }
							
	  ASKAPLOG_DEBUG_STR(logger,"Defining the image");
	  dim = new long;
	  dim[0] = dim[1] = 10;
	  image = new duchamp::Image(dim);
	  image->saveArray(array,100);
	  image->pars().setThreshold(5);
	  image->pars().setMinPix(1);

	  ASKAPLOG_DEBUG_STR(logger,"Finding the sources");
	  objlist = image->findSources2D();

	  fitparams = FittingParameters();
	  fitparams.setNumSubThresholds(5);

	}

	void tearDown() {
	  delete image;
	  delete array;
	};

	void findSource() {
	  ASKAPLOG_DEBUG_STR(logger, objlist.size());
	  CPPUNIT_ASSERT(objlist.size()==1);
	}

	/* void subthreshold() { */
	/*   ASKAPLOG_DEBUG_STR(logger,"Defining detection"); */
	/*   duchamp::Detection det; */
	/*   ASKAPLOG_DEBUG_STR(logger, objlist.size()); */
	/*   det.addChannel(0,objlist[0]); */
	/*   ASKAPLOG_DEBUG_STR(logger,"Defining radiosource"); */
	/*   itsSource = RadioSource(det); */
	/*   std::string secstring=duchamp::nullSection(2); */
	/*   duchamp::Section sec(secstring); */
	/*   itsSource.defineBox(sec,fitparams,2); */
					
	/*   ASKAPLOG_DEBUG_STR(logger,"Finding subcomponents"); */
	/*   sublist=itsSource.getSubComponentList(itsPos,itsF);					 */
					
	/*   CPPUNIT_ASSERT(sublist.size()==5); */
	/* } */

      };

    }

  }

}
