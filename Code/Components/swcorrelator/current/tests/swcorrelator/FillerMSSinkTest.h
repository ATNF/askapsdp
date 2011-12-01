/// @file
///
/// @brief Test of MS writing
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
/// @author Max Voronkov <Maxim.Voronkov@csiro.au>

#ifndef ASKAP_SWCORRELATOR_FILLER_MS_SINK_TEST_H
#define ASKAP_SWCORRELATOR_FILLER_MS_SINK_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <Common/ParameterSet.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>
#include <askap/AskapError.h>

// Class under test
#include <swcorrelator/FillerMSSink.h>

#include <swcorrelator/CorrProducts.h>

#include <string>

namespace askap {

namespace swcorrelator {

class FillerMSSinkTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FillerMSSinkTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testWrite);
  CPPUNIT_TEST_SUITE_END();
  LOFAR::ParameterSet itsParset;
public:
  void setUp() {
     itsParset = LOFAR::ParameterSet("./tests/swcorrelator/testmssink.in");
     std::string fname("test.ms");
     casa::File tmpFile(fname);
     if (tmpFile.exists()) {
         if (tmpFile.isDirectory()) {
             casa::Directory dir(fname);
             dir.removeRecursive();
         } else {
             ASKAPASSERT(tmpFile.isRegular());
             casa::RegularFile rf(fname);
             rf.remove();
         }         
     }          
  }
  
  void testCreate() {
     FillerMSSink sink(itsParset);
  }
  
  void testWrite() {
     FillerMSSink sink(itsParset);
     const int nchan = 16;
     const int nbeam = 9;
     for (int timestamp = 0; timestamp < 10; ++timestamp) {
          for (int beam = 9; beam < nbeam; ++beam) { 
              CorrProducts buf(nchan,beam);
              buf.itsVisibility.set(casa::Complex(4.,3.));
              buf.itsFlag.set(casa::False);
              buf.itsBAT = (4752000000ull + uint64_t(timestamp)) * 10000000ull;
              sink.write(buf);
          }
     }
  }
};

} // namespace swcorrelator

} // namespace askap

#endif // #ifndef ASKAP_SWCORRELATOR_FILLER_MS_SINK_TEST_H


