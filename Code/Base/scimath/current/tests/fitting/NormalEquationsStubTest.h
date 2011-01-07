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
      CPPUNIT_TEST_EXCEPTION(testMergeError, askap::AskapError);
      //CPPUNIT_TEST(testBlobStream);
      CPPUNIT_TEST_SUITE_END();

      private:
        boost::shared_ptr<NormalEquationsStub> itsNE;
        
      public:
        void setUp()
        { 
          itsNE.reset(new NormalEquationsStub);
        }
                        
        static casa::Matrix<double> populateMatrix(casa::uInt nrow, casa::uInt ncol,
                                            const double *buf)
        {
          casa::Matrix<double> result(nrow,ncol,0.);
          for (casa::uInt row = 0; row<nrow; ++row) {
               for (casa::uInt col = 0; col<ncol; ++col,++buf) {
                    result(row,col) = *buf;
               }
          }
          return result;
        }
        
        static casa::Vector<double> populateVector(casa::uInt size, 
                                                   const double *buf)
        {
          casa::Vector<double> result(size,0.);
          std::copy(buf,buf+size,result.cbegin());
          return result;
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

