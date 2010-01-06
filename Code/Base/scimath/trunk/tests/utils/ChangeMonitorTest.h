/// @file
///
/// Test of ChangeMonitor (helper class to assist keeping track of changes in e.g. parameters or
/// some other variable which can be cached)
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

#include <utils/ChangeMonitor.h>


namespace askap {

namespace scimath {

class ChangeMonitorTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(ChangeMonitorTest);
   CPPUNIT_TEST(testChangeMonitor);
   CPPUNIT_TEST_SUITE_END();
public:
   
   void testChangeMonitor() {
      ChangeMonitor cm;
      const ChangeMonitor cm2 = cm;
      CPPUNIT_ASSERT(cm2 == cm);
      cm.notifyOfChanges();
      CPPUNIT_ASSERT(cm2 != cm);
      for (size_t i=0; i<20; ++i) {
           cm.notifyOfChanges();
      }
      CPPUNIT_ASSERT(cm2 != cm);      
   }
};
    
} // namespace scimath

} // namespace askap

