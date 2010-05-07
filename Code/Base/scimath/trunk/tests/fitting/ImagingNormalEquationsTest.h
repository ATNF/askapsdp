/// @copyright (c) 2007 CSIRO
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

#include <fitting/ImagingNormalEquations.h>

#include <cppunit/extensions/HelperMacros.h>

#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>


#include <askap/AskapError.h>
#include <boost/shared_ptr.hpp>

#include <algorithm>

namespace askap
{
  namespace scimath
  {

    class ImagingNormalEquationsTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(ImagingNormalEquationsTest);
      CPPUNIT_TEST(testConstructors);
      CPPUNIT_TEST(testCopy);
      CPPUNIT_TEST(testFillMatrix);
      CPPUNIT_TEST(testMerge);
      CPPUNIT_TEST(testAdd);
      CPPUNIT_TEST(testCopySemantics);
#ifdef ASKAP_DEBUG
// the check is done and exception is thrown in the debug mode only
      CPPUNIT_TEST_EXCEPTION(testAddWrongDimension, askap::AskapError);
#endif // #ifdef ASKAP_DEBUG
      CPPUNIT_TEST(testBlobStream);
      CPPUNIT_TEST_SUITE_END();

      private:
        boost::shared_ptr<ImagingNormalEquations> p1, p2, p3, pempty;

      public:
        void setUp()
        { 
          p1.reset(new ImagingNormalEquations());
          p2.reset(new ImagingNormalEquations());
          p3.reset(new ImagingNormalEquations());
          pempty.reset(new ImagingNormalEquations());
        }

        void testConstructors()
        {
          Params ip;
          ip.add("Value0");
          ip.add("Value1");
          ip.add("Value2");
          p1.reset(new ImagingNormalEquations(ip));
          CPPUNIT_ASSERT(p1->parameters().names().size()==3);
          CPPUNIT_ASSERT(p1->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p1->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p1->parameters().names()[2]=="Value2");
          std::vector<std::string> params = p1->unknowns();
          CPPUNIT_ASSERT(params.size() == 3);
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value0") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value1") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value2") != params.end());
        }

        void testCopy()
        {
          Params ip;
          ip.add("Value0");
          ip.add("Value1");
          ip.add("Value2");
          p1.reset(new ImagingNormalEquations(ip));
          CPPUNIT_ASSERT(p1);
          p2.reset(new ImagingNormalEquations(*p1));
          CPPUNIT_ASSERT(p2->parameters().names().size()==3);
          CPPUNIT_ASSERT(p2->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value2");
          std::vector<std::string> params = p2->unknowns();
          CPPUNIT_ASSERT(params.size() == 3);
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value0") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value1") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value2") != params.end());
        }
        
        void testFillMatrix()
        {
          testCopy();
          CPPUNIT_ASSERT(p2);
          p2->addSlice("Value1", casa::Vector<double>(5,0.1), 
                  casa::Vector<double>(5, 1.), casa::Vector<double>(5,-40.),
                  casa::IPosition(1,0));
          testAllElements(extractVector(p2->normalMatrixDiagonal(), "Value1"),5,1.);
          testAllElements(extractVector(p2->normalMatrixSlice(), "Value1"),5,0.1);
          testAllElements(extractVector(p2->dataVector(), "Value1"),5,-40.);
          testAllElements(p2->dataVector("Value1"),5,-40.);
          
          p2->addDiagonal("Value2", casa::Vector<double>(3, 1.), casa::Vector<double>(3, 10.));
          testAllElements(extractVector(p2->normalMatrixDiagonal(), "Value2"),3,1.);
          testAllElements(extractVector(p2->normalMatrixSlice(), "Value2"),0,0.);
          testAllElements(extractVector(p2->dataVector(), "Value2"),3,10.);
          testAllElements(p2->dataVector("Value2"),3,10.);
          
          std::vector<std::string> params = p2->unknowns();
          CPPUNIT_ASSERT(params.size() == 3);
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value0") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value1") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value2") != params.end());                    
        }
        
        void testCopySemantics()
        {
          testFillMatrix();
          CPPUNIT_ASSERT(p2);
          p3 = boost::dynamic_pointer_cast<ImagingNormalEquations>(p2->clone());
          CPPUNIT_ASSERT(p3);

          // test that values are as expected after testFillMatrix
          testAllElements(extractVector(p2->normalMatrixDiagonal(), "Value1"),5,1.);
          testAllElements(extractVector(p2->normalMatrixSlice(), "Value1"),5,0.1);
          testAllElements(extractVector(p2->dataVector(), "Value1"),5,-40.);
          testAllElements(p2->dataVector("Value1"),5,-40.);
          testAllElements(extractVector(p2->normalMatrixDiagonal(), "Value2"),3,1.);
          testAllElements(extractVector(p2->normalMatrixSlice(), "Value2"),0,0.);
          testAllElements(extractVector(p2->dataVector(), "Value2"),3,10.);
          testAllElements(p2->dataVector("Value2"),3,10.);

          // test that copy worked
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value1"),5,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value1"),5,0.1);
          testAllElements(extractVector(p3->dataVector(), "Value1"),5,-40.);
          testAllElements(p3->dataVector("Value1"),5,-40.);
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value2"),3,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value2"),0,0.);
          testAllElements(extractVector(p3->dataVector(), "Value2"),3,10.);
          testAllElements(p3->dataVector("Value2"),3,10.);
          
          // change original values
          p2->addSlice("Value1", casa::Vector<double>(5,0.1), 
                  casa::Vector<double>(5, 1.), casa::Vector<double>(5,-40.),
                  casa::IPosition(1,0));
          p2->addDiagonal("Value2", casa::Vector<double>(3, 1.), casa::Vector<double>(3, 10.));
          
          // test that they were indeed changed
          testAllElements(extractVector(p2->normalMatrixDiagonal(), "Value1"),5,2.);
          testAllElements(extractVector(p2->normalMatrixSlice(), "Value1"),5,0.2);
          testAllElements(extractVector(p2->dataVector(), "Value1"),5,-80.);
          testAllElements(p2->dataVector("Value1"),5,-80.);
          testAllElements(extractVector(p2->normalMatrixDiagonal(), "Value2"),3,2.);
          testAllElements(extractVector(p2->normalMatrixSlice(), "Value2"),0,0.);
          testAllElements(extractVector(p2->dataVector(), "Value2"),3,20.);
          testAllElements(p2->dataVector("Value2"),3,20.);
          
          // test that the cloned equations have the old values
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value1"),5,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value1"),5,0.1);
          testAllElements(extractVector(p3->dataVector(), "Value1"),5,-40.);
          testAllElements(p3->dataVector("Value1"),5,-40.);
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value2"),3,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value2"),0,0.);
          testAllElements(extractVector(p3->dataVector(), "Value2"),3,10.);
          testAllElements(p3->dataVector("Value2"),3,10.);          
        }
        
        void testMerge() 
        {
          testFillMatrix();
          CPPUNIT_ASSERT(p3);
          CPPUNIT_ASSERT(p2);
          p3->merge(*p2);
          CPPUNIT_ASSERT(p3->parameters().names().size()==3);
          CPPUNIT_ASSERT(p3->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p3->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p3->parameters().names()[2]=="Value2");           
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value1"),5,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value1"),5,0.1);
          testAllElements(p3->dataVector("Value1"),5,-40.);          

          CPPUNIT_ASSERT(pempty);
          p3->merge(*pempty);
          CPPUNIT_ASSERT(p3->parameters().names().size()==3);
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value1"),5,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value1"),5,0.1);
          testAllElements(p3->dataVector("Value1"),5,-40.);
          
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value2"),3,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value2"),0,0.);
          testAllElements(p3->dataVector("Value2"),3,10.);
                    

          Params ip;
          ip.add("Value1");
          ip.add("Value4");
          p1.reset(new ImagingNormalEquations(ip));
          CPPUNIT_ASSERT(p1);
          p1->addSlice("Value1", casa::Vector<double>(5,0.), 
                  casa::Vector<double>(5, 1.), casa::Vector<double>(5,10.),
                  casa::IPosition(1,0));
          
          p3->merge(*p1);
          CPPUNIT_ASSERT(p3->parameters().names().size()==4);
          CPPUNIT_ASSERT(p3->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p3->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p3->parameters().names()[2]=="Value2");           
          CPPUNIT_ASSERT(p3->parameters().names()[3]=="Value4");
          
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value1"),5,2.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value1"),5,0.1);
          testAllElements(p3->dataVector("Value1"),5,-30.);                    
        
          std::vector<std::string> params = p3->unknowns();
          CPPUNIT_ASSERT(params.size() == 4);
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value0") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value1") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value2") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value4") != params.end());
          
          // Value2 should not change
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value2"),3,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value2"),0,0.);
          testAllElements(p3->dataVector("Value2"),3,10.);

          Params ip2;
          ip2.add("Value2");
          p2.reset(new ImagingNormalEquations(ip2));
          CPPUNIT_ASSERT(p2);
          p2->addSlice("Value2", casa::Vector<double>(7,-0.1), 
                  casa::Vector<double>(7, 1.), casa::Vector<double>(7,-10.),
                  casa::IPosition(1,0));
          
          //now Value2 is expected to be overwritten, because the shape has been changed
          p3->merge(*p2);
          CPPUNIT_ASSERT(p3->parameters().names().size()==4);
          CPPUNIT_ASSERT(p3->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p3->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p3->parameters().names()[2]=="Value2");           
          CPPUNIT_ASSERT(p3->parameters().names()[3]=="Value4");

          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value1"),5,2.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value1"),5,0.1);
          testAllElements(p3->dataVector("Value1"),5,-30.);                    

          // test for new Value2    
          testAllElements(extractVector(p3->normalMatrixDiagonal(), "Value2"),7,1.);
          testAllElements(extractVector(p3->normalMatrixSlice(), "Value2"),7,-0.1);
          testAllElements(p3->dataVector("Value2"),7,-10.);
        }

        void testAdd() 
        {
          testFillMatrix();
          CPPUNIT_ASSERT(p2);
          
          // add a slice with the same dimension
          p2->addSlice("Value1", casa::Vector<double>(5,0.2), 
                  casa::Vector<double>(5, 1.1), casa::Vector<double>(5,30.),
                  casa::IPosition(1,0));
          testAllElements(extractVector(p2->normalMatrixDiagonal(), "Value1"),5,2.1);
          testAllElements(extractVector(p2->normalMatrixSlice(), "Value1"),5,0.3);
          testAllElements(p2->dataVector("Value1"),5,-10.);
          
          // add diagonal with the same dimension
          p2->addDiagonal("Value2", casa::Vector<double>(3, 0.9), casa::Vector<double>(3, 1.));
          testAllElements(extractVector(p2->normalMatrixDiagonal(), "Value2"),3,1.9);
          testAllElements(extractVector(p2->normalMatrixSlice(), "Value2"),0,0.);
          testAllElements(p2->dataVector("Value2"),3,11.);          
         
          std::vector<std::string> params = p2->unknowns();
          CPPUNIT_ASSERT(params.size() == 3);
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value0") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value1") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value2") != params.end());                                                            
        }

        void testAddWrongDimension() 
        {
          // in the debug mode we check whether the dimesnions of added slices and diagonals
          // are consistent. This test is executed in debug mode only!
          testAdd();
          CPPUNIT_ASSERT(p2);
          
          // now add slice with the different dimension to check that it generates an exception
          p2->addSlice("Value1", casa::Vector<double>(7,0.2), 
                  casa::Vector<double>(5, 1.1), casa::Vector<double>(7,30.),
                  casa::IPosition(1,0));
        }
        
        void testBlobStream() {
          Params ip;
          ip.add("Value0");
          ip.add("Value1", 1.5);
          uint imsize=10*10;
          casa::Vector<double> im(imsize);
          im.set(3.0);
          ip.add("Image2", im);
          p1.reset(new ImagingNormalEquations(ip));
          CPPUNIT_ASSERT(p1);
          LOFAR::BlobString b1(false);
          LOFAR::BlobOBufString bob(b1);
          LOFAR::BlobOStream bos(bob);
          bos << *p1;
          Params pnew;
          LOFAR::BlobIBufString bib(b1);
          LOFAR::BlobIStream bis(bib);
          p2.reset(new ImagingNormalEquations());
          CPPUNIT_ASSERT(p2);
          bis >> *p2;
          CPPUNIT_ASSERT(p2->parameters().names().size()==3);
          CPPUNIT_ASSERT(p2->parameters().names()[0]=="Image2");
          CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value0");
          CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value1");

          std::vector<std::string> params = p2->unknowns();
          CPPUNIT_ASSERT(params.size() == 3);
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value0") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Value1") != params.end());
          CPPUNIT_ASSERT(std::find(params.begin(),params.end(),"Image2") != params.end());                                                            
        }
    protected:
        /// @brief a helper method to access map elements
        /// @details This method extracts a casa::Vector out of the map
        /// doing all necessary checks.
        /// @param[in] inMap input map passed by const reference
        /// @param[in] key string key
        /// @return casa::Vector corresponding to the given key
        static const casa::Vector<double> extractVector(const std::map<std::string, 
               casa::Vector<double> > &inMap, const std::string &key) {
          std::map<std::string, casa::Vector<double> >::const_iterator ci = inMap.find(key);
          CPPUNIT_ASSERT(ci != inMap.end());
          return ci->second;
        } 
        
        /// @brief test values stored in a vector
        /// @details This method encapsulates a loop over vector and tests
        /// all its elements against given value. The length is also tested.
        /// @param[in] vec input vector (passed by const reference)
        /// @param[in] expectedSize expected size of the vector
        /// @param[in] expectedValue expected value of all elements
        static void testAllElements(const casa::Vector<double> &vec,
                casa::uInt expectedSize, double expectedValue) {
          CPPUNIT_ASSERT(vec.nelements() == expectedSize);
          for (casa::uInt i=0; i<expectedSize; ++i) {
               CPPUNIT_ASSERT(fabs(vec[i]-expectedValue)<1e-6);
          } 
        } 
    };

  }
}
