/// @file
///
/// Unit test for the CASA image access code
///
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

#include <imageaccess/ImageAccessFactory.h>
#include <cppunit/extensions/HelperMacros.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/IPosition.h>

#include <boost/shared_ptr.hpp>

#include <APS/ParameterSet.h>


namespace askap {

namespace synthesis {

class CasaImageAccessTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(CasaImageAccessTest);
   CPPUNIT_TEST(testReadWrite);
   CPPUNIT_TEST_SUITE_END();
public:
   void setUp() {
      LOFAR::ACC::APS::ParameterSet parset;
      parset.add("imagetype","casa");
      itsImageAccessor = imageAccessFactory(parset);
   }
   
   void testReadWrite() {
   }
   
   
private:
   /// @brief method to access image
   boost::shared_ptr<IImageAccess> itsImageAccessor;         
};
    
} // namespace synthesis

} // namespace askap

