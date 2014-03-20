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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <preprocessing/VariableThresholdingHelpers.h>
#include <cppunit/extensions/HelperMacros.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <duchamp/Utils/Statistics.hh>

ASKAP_LOGGER(logger, ".slidingMathTest");
namespace askap {
  namespace analysis {

    const size_t dim=8;
    const size_t boxWidth=1;
 
    class SlidingMathTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE(SlidingMathTest);
      CPPUNIT_TEST(testBoxMean);
      CPPUNIT_TEST(testBoxStddev);
      CPPUNIT_TEST(testBoxMedian);
      CPPUNIT_TEST(testBoxMadfm);
      CPPUNIT_TEST_SUITE_END();

    private:
      casa::Array<Float> inputArr;
      casa::Array<Float> checkBoxMeanArr;
      casa::Array<Float> checkBoxMedianArr;
      casa::Array<Float> checkBoxStddevArr;
      casa::Array<Float> checkBoxMadfmArr;
      casa::IPosition shape;
    public:

      void setUp() {
	
	const size_t arrsize=dim*dim;
	float input[arrsize] = { 1,2,3,4,5,6,7,8,
				 9,10,11,12,13,14,15,16,
				 17,18,19,20,21,22,23,24,
				 25,26,27,28,29,30,31,32,
				 33,34,35,36,37,38,39,40,
				 41,42,43,44,45,46,47,48,
				 49,50,51,52,53,54,55,56,
				 57,58,59,60,61,62,63,64};
	shape = casa::IPosition(2,dim,dim);
	inputArr.takeStorage(shape,input);
	float checkBoxMean[arrsize] = {0,0,0,0,0,0,0,0,
				       0,10,11,12,13,14,15,0,
				       0,18,19,20,21,22,23,0,
				       0,26,27,28,29,30,31,0,
				       0,34,35,36,37,38,39,0,
				       0,42,43,44,45,46,47,0,
				       0,50,51,52,53,54,55,0,
				       0,0,0,0,0,0,0,0};
	checkBoxMeanArr.takeStorage(shape,checkBoxMean);
	checkBoxMedianArr.takeStorage(shape,checkBoxMean);
	float base[arrsize] = {0,0,0,0,0,0,0,0,
			       0,1,1,1,1,1,1,0,
			       0,1,1,1,1,1,1,0,
			       0,1,1,1,1,1,1,0,
			       0,1,1,1,1,1,1,0,
			       0,1,1,1,1,1,1,0,
			       0,1,1,1,1,1,1,0,
			       0,0,0,0,0,0,0,0};
	checkBoxStddevArr.takeStorage(shape,base);
	float stddevVal = sqrt(2.*(7*7+8*8+9*9+1)/8.);
	checkBoxStddevArr *= stddevVal;
	checkBoxMadfmArr.takeStorage(shape,base);
	float madfmVal = 7. / Statistics::correctionFactor;
	checkBoxMadfmArr *= madfmVal;
	/* float checkBoxStddev[arrsize] = {0,0,0,0,0,0,0,0, */
      }

      void testBoxMean()
      {
	casa::IPosition box(2,boxWidth,boxWidth);
	casa::Array<Float> middle(shape,0.);
	casa::Array<Float> spread(shape,0.);
	size_t dimMin=0;
	size_t dimMax=dim;
	/* size_t dimMin=boxWidth; */
	/* size_t dimMax=dim-boxWidth; */
	casa::Array<Float> localInput(inputArr);
	ASKAPLOG_DEBUG_STR(logger, "Sliding math test - mean");
	slidingBoxStats(localInput, middle, spread, box, false);
	for(size_t y=dimMin;y<dimMax;y++){
	  for(size_t x=dimMin;x<dimMax;x++){
	    casa::IPosition pos(2,x,y);
	    CPPUNIT_ASSERT(fabs(middle(pos)-checkBoxMeanArr(pos))<1.e-5);
	  }
	}
	ASKAPLOG_DEBUG_STR(logger, "Confirming input unchanged");
	for(size_t y=dimMin;y<dimMax;y++){
	  for(size_t x=dimMin;x<dimMax;x++){
	    casa::IPosition pos(2,x,y);
	    CPPUNIT_ASSERT(fabs(inputArr(pos)-localInput(pos))<1.e-5);
	  }
	}
      }

      void testBoxStddev()
      {
	casa::IPosition box(2,boxWidth,boxWidth);
	casa::Array<Float> middle(shape,0.);
	casa::Array<Float> spread(shape,0.);
	size_t dimMin=0;
	size_t dimMax=dim;
	/* size_t dimMin=boxWidth; */
	/* size_t dimMax=dim-boxWidth; */
	casa::Array<Float> localInput(inputArr);
	ASKAPLOG_DEBUG_STR(logger, "Sliding math test - stddev");
	slidingBoxStats(localInput, middle, spread, box, false);
	ASKAPLOG_DEBUG_STR(logger, "Calculated stddev follows: " << spread);
	ASKAPLOG_DEBUG_STR(logger, "Should be: " << checkBoxStddevArr);
	for(size_t y=dimMin;y<dimMax;y++){
	  for(size_t x=dimMin;x<dimMax;x++){
	    casa::IPosition pos(2,x,y);
	    CPPUNIT_ASSERT(fabs(middle(pos)-checkBoxMeanArr(pos))<1.e-5);
	  }
	}
	ASKAPLOG_DEBUG_STR(logger, "Confirming input unchanged");
	for(size_t y=dimMin;y<dimMax;y++){
	  for(size_t x=dimMin;x<dimMax;x++){
	    casa::IPosition pos(2,x,y);
	    CPPUNIT_ASSERT(fabs(inputArr(pos)-localInput(pos))<1.e-5);
	  }
	}
      }

      void testBoxMedian()
      {
	casa::IPosition box(2,boxWidth,boxWidth);
	casa::Array<Float> middle(shape,0.);
	casa::Array<Float> spread(shape,0.);
	size_t dimMin=0;
	size_t dimMax=dim;
	ASKAPLOG_DEBUG_STR(logger, "Sliding math test - median");
	casa::Array<Float> localInput(inputArr);
	slidingBoxStats(localInput, middle, spread, box, true);
	ASKAPLOG_DEBUG_STR(logger, "Calculated median follows: " << middle);
	ASKAPLOG_DEBUG_STR(logger, "Should be: " << checkBoxMedianArr);
	for(size_t y=dimMin;y<dimMax;y++){
	  for(size_t x=dimMin;x<dimMax;x++){
	    casa::IPosition pos(2,x,y);
	    CPPUNIT_ASSERT(fabs(middle(pos)-checkBoxMedianArr(pos))<1.e-5);
	  }
	}
	ASKAPLOG_DEBUG_STR(logger, "Confirming input unchanged");
	for(size_t y=dimMin;y<dimMax;y++){
	  for(size_t x=dimMin;x<dimMax;x++){
	    casa::IPosition pos(2,x,y);
	    CPPUNIT_ASSERT(fabs(inputArr(pos)-localInput(pos))<1.e-5);
	  }
	}

      }

      void testBoxMadfm()
      {
	casa::IPosition box(2,boxWidth,boxWidth);
	casa::Array<Float> middle(shape,0.);
	casa::Array<Float> spread(shape,0.);
	size_t dimMin=0;
	size_t dimMax=dim;
	ASKAPLOG_DEBUG_STR(logger, "Sliding math test - median");
	casa::Array<Float> localInput(inputArr);
	slidingBoxStats(localInput, middle, spread, box, true);
	ASKAPLOG_DEBUG_STR(logger, "Calculated madfm follows: " << spread);
	ASKAPLOG_DEBUG_STR(logger, "Should be: " << checkBoxMadfmArr);
	for(size_t y=dimMin;y<dimMax;y++){
	  for(size_t x=dimMin;x<dimMax;x++){
	    casa::IPosition pos(2,x,y);
	    CPPUNIT_ASSERT(fabs(spread(pos)-checkBoxMadfmArr(pos))<1.e-5);
	  }
	}
	ASKAPLOG_DEBUG_STR(logger, "Confirming input unchanged");
	for(size_t y=dimMin;y<dimMax;y++){
	  for(size_t x=dimMin;x<dimMax;x++){
	    casa::IPosition pos(2,x,y);
	    CPPUNIT_ASSERT(fabs(inputArr(pos)-localInput(pos))<1.e-5);
	  }
	}

      }

      void tearDown() {
      }

    };

  }
}
