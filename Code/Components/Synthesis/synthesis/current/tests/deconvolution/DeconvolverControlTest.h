/// @file
///
/// Unit test for the deconvolution control class
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>

#include <deconvolution/DeconvolverState.h>
#include <deconvolution/DeconvolverControl.h>
#include <cppunit/extensions/HelperMacros.h>

#include <casa/BasicSL/Complex.h>

#include <boost/shared_ptr.hpp>

using namespace casa;

namespace askap {

namespace synthesis {

class DeconvolverControlTest : public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(DeconvolverControlTest);
   CPPUNIT_TEST(testTermination);
   CPPUNIT_TEST_SUITE_END();
public:
   
   void testTermination() {
     {
       DeconvolverState<Float> ds;
       CPPUNIT_ASSERT(!itsDC.terminate(ds));
       ds.setCurrentIter(100);
       CPPUNIT_ASSERT(!itsDC.terminate(ds));
       ds.setEndIter(200);
       CPPUNIT_ASSERT(!itsDC.terminate(ds));
       ds.setCurrentIter(199);
       CPPUNIT_ASSERT(!itsDC.terminate(ds));
       ds.setCurrentIter(200);
       CPPUNIT_ASSERT(itsDC.terminate(ds));
       ds.setCurrentIter(300);
       CPPUNIT_ASSERT(itsDC.terminate(ds));
       CPPUNIT_ASSERT(itsDC.terminationCause()==DeconvolverControl<Float>::EXCEEDEDITERATIONS);
     }
   }
   
private:
   /// @brief DeconvolutionControl class
   DeconvolverControl<Float> itsDC;
};
    
} // namespace synthesis

} // namespace askap

