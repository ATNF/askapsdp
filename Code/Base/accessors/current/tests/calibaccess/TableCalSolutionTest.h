/// @file
///
/// Unit test for the table-based implementation of the interface to access
/// calibration solutions
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

#include <casa/aipstype.h>
#include <cppunit/extensions/HelperMacros.h>
#include <calibaccess/TableCalSolutionSource.h>
#include <calibaccess/ParsetCalSolutionAccessor.h>
#include <calibaccess/JonesIndex.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableError.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>



#include <boost/shared_ptr.hpp>
#include <string>

namespace askap {

namespace accessors {

class TableCalSolutionTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(TableCalSolutionTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST_SUITE_END();
protected:
   bool tableExists(const std::string &fname) {
       try {
          casa::Table testTab(fname,casa::Table::Old);
          testTab.throwIfNull();
       } 
       catch (const casa::TableError &) {
          return false;
       }
       return true; 
   }

   boost::shared_ptr<ICalSolutionSource> rwSource(bool doRemove) {
       const std::string fname("calibdata.tab");
       if (doRemove) {
           if (casa::Table::canDeleteTable(fname,false)) {
               casa::Table::deleteTable(fname, false);
           } else {
              // check that the table simply doesn't exist
              CPPUNIT_ASSERT(!tableExists(fname));
              casa::File tmpFile(fname);
              if (tmpFile.exists()) {
                  // we need to remove the file with the given name
                  if (tmpFile.isDirectory()) {
                      casa::Directory dir(fname);
                      dir.remove();
                  } else {
                      CPPUNIT_ASSERT(tmpFile.isRegular());
                      casa::RegularFile rf(fname);
                      rf.remove();
                  }
              }
           }
       }
       boost::shared_ptr<TableCalSolutionSource> css(new TableCalSolutionSource(fname,6,3,8));
       CPPUNIT_ASSERT(css);
       return css;
   }
public:

   void testCreate() {
       boost::shared_ptr<ICalSolutionSource> css = rwSource(true);
       long newID = css->newSolutionID(0.);
       CPPUNIT_ASSERT_EQUAL(0l, newID);
       boost::shared_ptr<ICalSolutionAccessor> acc = css->rwSolution(newID);
       acc->setGain(JonesIndex(0u,0u),JonesJTerm(casa::Complex(1.0,-1.0),true,casa::Complex(-1.0,1.0),true));
       // reuse existing table
       acc.reset();
       css.reset();
       css = rwSource(false);
       newID = css->newSolutionID(60.);
       CPPUNIT_ASSERT_EQUAL(1l, newID);
       acc = css->rwSolution(newID);
       acc->setLeakage(JonesIndex(2u,1u),JonesDTerm(casa::Complex(0.1,-0.1),true,casa::Complex(-0.1,0.4),false));
       // once again reuse the table
       acc.reset();
       css.reset();
       css = rwSource(false);
       newID = css->newSolutionID(120.);
       CPPUNIT_ASSERT_EQUAL(2l, newID);
       acc = css->rwSolution(newID);
       acc->setBandpass(JonesIndex(1u,1u),JonesJTerm(casa::Complex(1.0,-0.2),true,casa::Complex(0.9,-0.1),true),1u);       
   }
};

} // namespace accessors

} // namespace askap

