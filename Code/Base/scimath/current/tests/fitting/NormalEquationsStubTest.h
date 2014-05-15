/// @file
/// 
/// @brief This file contains tests of normal equations stub
///
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef NORMAL_EQUATIONS_STUB_TEST_H
#define NORMAL_EQUATIONS_STUB_TEST_H

#include <fitting/NormalEquationsStub.h>
#include <fitting/GenericNormalEquations.h>
#include <fitting/DesignMatrix.h>

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

    class NormalEquationsStubTest : public CppUnit::TestFixture 
    {
 
      CPPUNIT_TEST_SUITE(NormalEquationsStubTest);
      CPPUNIT_TEST(testMerge);
      CPPUNIT_TEST(testGeneralChecks);
      CPPUNIT_TEST_EXCEPTION(testMergeError, askap::AskapError);
      CPPUNIT_TEST_EXCEPTION(testDataVectorError, askap::AskapError);
      CPPUNIT_TEST_EXCEPTION(testNormalMatrixError, askap::AskapError);
      CPPUNIT_TEST(testBlobStream);
      CPPUNIT_TEST_SUITE_END();

      private:
        boost::shared_ptr<NormalEquationsStub> itsNE;
        
      public:
        void setUp()
        { 
          itsNE.reset(new NormalEquationsStub);
        }
        
        void testGeneralChecks()
        {
          CPPUNIT_ASSERT(itsNE);
          boost::shared_ptr<NormalEquationsStub> bufNE = boost::dynamic_pointer_cast<NormalEquationsStub>(itsNE->clone());
          CPPUNIT_ASSERT(bufNE);
          bufNE->reset();
          const std::vector<std::string> names = itsNE->unknowns();
          CPPUNIT_ASSERT(names.size() == 0);
          // checking that the stub can't be converted to either generic or imaging-specific normal equations
          // by mistake
          boost::shared_ptr<GenericNormalEquations> gne = boost::dynamic_pointer_cast<GenericNormalEquations>(itsNE);
          CPPUNIT_ASSERT(!gne);
          boost::shared_ptr<ImagingNormalEquations> ine = boost::dynamic_pointer_cast<ImagingNormalEquations>(itsNE);
          CPPUNIT_ASSERT(!ine);
        }
                                        
        void testMerge() 
        {
          boost::shared_ptr<NormalEquationsStub> bufNE(new NormalEquationsStub);
          CPPUNIT_ASSERT(bufNE);
        }

        void testMergeError()
        {
          boost::shared_ptr<GenericNormalEquations> bufNE(new GenericNormalEquations);
          const casa::uInt nData = 10;
          DesignMatrix dm;
          dm.addDerivative("Value0", casa::Matrix<casa::Double>(nData, 1, 1.0));
          dm.addDerivative("Value1", casa::Matrix<casa::Double>(nData, 1, 2.0));
          dm.addResidual(casa::Vector<casa::Double>(nData, -1.0), casa::Vector<double>(nData, 1.0));
          CPPUNIT_ASSERT(dm.nData() == nData);
          CPPUNIT_ASSERT(bufNE);
          bufNE->add(dm);
          
          itsNE->merge(*bufNE);
        }
        
        void testDataVectorError()
        {
          CPPUNIT_ASSERT(itsNE);
          const casa::Vector<double>& vec = itsNE->dataVector("Value0"); 
          CPPUNIT_ASSERT(vec.size() == 0);
        }

        void testNormalMatrixError()
        {
          CPPUNIT_ASSERT(itsNE);
          const casa::Matrix<double>& nm = itsNE->normalMatrix("Value0","Value1"); 
          CPPUNIT_ASSERT(nm.nelements() == 0);
        }
        
        void testBlobStream()
        {
          LOFAR::BlobString bstr(false);
          LOFAR::BlobOBufString bob(bstr);
          LOFAR::BlobOStream bos(bob);
          
          bos<<*itsNE;
          
          itsNE->reset();
          
          LOFAR::BlobIBufString bib(bstr);
          LOFAR::BlobIStream bis(bib);
          bis>>*itsNE;          
        }
    };

  }
}

#endif // #ifndef NORMAL_EQUATIONS_STUB_TEST_H

