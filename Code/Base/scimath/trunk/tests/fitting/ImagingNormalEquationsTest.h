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
          
        }
        
        void testMerge() 
        {
          testCopy();
          CPPUNIT_ASSERT(p3);
          CPPUNIT_ASSERT(p2);
          p3->merge(*p2);
          CPPUNIT_ASSERT(p3->parameters().names().size()==3);
          CPPUNIT_ASSERT(p3->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p3->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p3->parameters().names()[2]=="Value2");           
          CPPUNIT_ASSERT(pempty);
          p3->merge(*pempty);
          CPPUNIT_ASSERT(p3->parameters().names().size()==3);

          Params ip;
          ip.add("Value1");
          ip.add("Value4");
          p1.reset(new ImagingNormalEquations(ip));
          CPPUNIT_ASSERT(p1);
          p3->merge(*p1);
          CPPUNIT_ASSERT(p3->parameters().names().size()==4);
          CPPUNIT_ASSERT(p3->parameters().names()[0]=="Value0");
          CPPUNIT_ASSERT(p3->parameters().names()[1]=="Value1");
          CPPUNIT_ASSERT(p3->parameters().names()[2]=="Value2");           
          CPPUNIT_ASSERT(p3->parameters().names()[3]=="Value4");
          
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
          CPPUNIT_ASSERT(p2->parameters().names()[0]=="Image2");
          CPPUNIT_ASSERT(p2->parameters().names().size()==3);
          CPPUNIT_ASSERT(p2->parameters().names()[1]=="Value0");
          CPPUNIT_ASSERT(p2->parameters().names()[2]=="Value1");
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
