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

#include <cppunit/extensions/HelperMacros.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/LinearCoordinate.h>


#include <boost/shared_ptr.hpp>

#include <Common/ParameterSet.h>


namespace askap {

namespace synthesis {

class CasaProjectionTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(CasaProjectionTest);
   CPPUNIT_TEST(testSINProjection);
   CPPUNIT_TEST_SUITE_END();
public:
   void setUp() {
   }
   
   void testSINProjection() {
   }
   
};
    
} // namespace synthesis

} // namespace askap

