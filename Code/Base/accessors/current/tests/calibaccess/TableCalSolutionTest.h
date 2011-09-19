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



#include <boost/shared_ptr.hpp>
#include <string>

namespace askap {

namespace accessors {

class TableCalSolutionTest : public CppUnit::TestFixture 
{
   CPPUNIT_TEST_SUITE(TableCalSolutionTest);
   CPPUNIT_TEST(testCreate);
   CPPUNIT_TEST(testRead);
   CPPUNIT_TEST_EXCEPTION(testUndefinedGains, AskapError);
   CPPUNIT_TEST_EXCEPTION(testUndefinedLeakages, AskapError);
   CPPUNIT_TEST_EXCEPTION(testUndefinedBandpasses, AskapError);
   CPPUNIT_TEST_EXCEPTION(testUndefinedSolution, AskapError);
   CPPUNIT_TEST_SUITE_END();
protected:

   static boost::shared_ptr<ICalSolutionSource> rwSource(bool doRemove) {
       const std::string fname("calibdata.tab");
       if (doRemove) {
           TableCalSolutionSource::removeOldTable(fname);
       }
       boost::shared_ptr<TableCalSolutionSource> css(new TableCalSolutionSource(fname,6,3,8));
       CPPUNIT_ASSERT(css);
       return css;
   }
   
   static boost::shared_ptr<ICalSolutionConstSource> roSource() {
       const std::string fname("calibdata.tab");
       boost::shared_ptr<TableCalSolutionConstSource> css(new TableCalSolutionConstSource(fname));
       CPPUNIT_ASSERT(css);
       return css;
   }
   
   static void testComplex(const casa::Complex &expected, const casa::Complex &obtained, const float tol = 1e-5) {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(real(expected),real(obtained),tol);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(imag(expected),imag(obtained),tol);      
   }
   
   static boost::shared_ptr<ICalSolutionConstAccessor> accessorForExistingTable() {
       const boost::shared_ptr<ICalSolutionConstSource> css = roSource();
       CPPUNIT_ASSERT(css);
       const long sID = css->mostRecentSolution();
       CPPUNIT_ASSERT_EQUAL(2l, sID);
       const boost::shared_ptr<ICalSolutionConstAccessor> acc = css->roSolution(sID);
       CPPUNIT_ASSERT(acc);
       return acc;
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
   
   void testRead() {
       // rerun the code creating a table, although we could've just relied on the fact that testCreate() is executed
       // just before this test
       testCreate();
       const boost::shared_ptr<ICalSolutionConstSource> css = roSource();
       CPPUNIT_ASSERT(css);
       const long sID = css->mostRecentSolution();
       CPPUNIT_ASSERT_EQUAL(2l, sID);
       for (long id = 0; id<3; ++id) {
            CPPUNIT_ASSERT_EQUAL(id, css->solutionID(0.5+60.*id));
       }
       const boost::shared_ptr<ICalSolutionConstAccessor> acc = css->roSolution(sID);
       CPPUNIT_ASSERT(acc);
       // test gains
       for (casa::uInt ant = 0; ant<6; ++ant) {
            for (casa::uInt beam = 0; beam<3; ++beam) {
                 const JonesJTerm gain = acc->gain(JonesIndex(ant,beam));
                 if ((ant == 0) && (beam == 0)) {
                     testComplex(casa::Complex(1.,-1.), gain.g1());
                     testComplex(casa::Complex(-1.,1.), gain.g2());
                     CPPUNIT_ASSERT(gain.g1IsValid());
                     CPPUNIT_ASSERT(gain.g2IsValid());                               
                 } else {
                     // default gain is 1.0
                     testComplex(casa::Complex(1.0,0.), gain.g1());
                     testComplex(casa::Complex(1.0,0.), gain.g2());
                     CPPUNIT_ASSERT(!gain.g1IsValid());
                     CPPUNIT_ASSERT(!gain.g2IsValid());                                                    
                 }
            }
       }
       // test leakages
       for (casa::uInt ant = 0; ant<6; ++ant) {
            for (casa::uInt beam = 0; beam<3; ++beam) {
                 const JonesDTerm leakage = acc->leakage(JonesIndex(ant,beam));
                 if ((ant == 2) && (beam == 1)) {
                     testComplex(casa::Complex(0.1,-0.1), leakage.d12());
                     testComplex(casa::Complex(-0.1,0.4), leakage.d21());
                     CPPUNIT_ASSERT(leakage.d12IsValid());
                     CPPUNIT_ASSERT(!leakage.d21IsValid());                               
                 } else {
                     // default leakage is 0.0
                     testComplex(casa::Complex(0.), leakage.d12());
                     testComplex(casa::Complex(0.), leakage.d21());
                     CPPUNIT_ASSERT(!leakage.d12IsValid());
                     CPPUNIT_ASSERT(!leakage.d21IsValid());                                                    
                 }
            }
       }
       // test bandpasses
       for (casa::uInt ant = 0; ant<6; ++ant) {
            for (casa::uInt beam = 0; beam<3; ++beam) {
                 const JonesIndex index(ant,beam);
                 for (casa::uInt chan = 0; chan < 8; ++chan) {
                      const JonesJTerm bp = acc->bandpass(index,chan);
                      if ((ant == 1) && (beam == 1) && (chan == 1)) {
                          testComplex(casa::Complex(1.0,-0.2), bp.g1());
                          testComplex(casa::Complex(0.9,-0.1), bp.g2());
                          CPPUNIT_ASSERT(bp.g1IsValid());
                          CPPUNIT_ASSERT(bp.g2IsValid());                               
                      } else {
                          // default bandpass gain is 1.0
                          testComplex(casa::Complex(1.0,0.), bp.g1());
                          testComplex(casa::Complex(1.0,0.), bp.g2());
                          CPPUNIT_ASSERT(!bp.g1IsValid());
                          CPPUNIT_ASSERT(!bp.g2IsValid());                                                    
                      }
                 }
            }
       }       
   }
   
   void testUndefinedGains() {
       const boost::shared_ptr<ICalSolutionConstAccessor> acc = accessorForExistingTable();
       CPPUNIT_ASSERT(acc);
       // only 6 antennas, 3 beams and 8 channels are defined
       acc->gain(JonesIndex(7u,0u));
   }

   void testUndefinedLeakages() {
       const boost::shared_ptr<ICalSolutionConstAccessor> acc = accessorForExistingTable();
       CPPUNIT_ASSERT(acc);
       // only 6 antennas, 3 beams and 8 channels are defined
       acc->leakage(JonesIndex(3u,3u));
   }
   
   void testUndefinedBandpasses() {
       const boost::shared_ptr<ICalSolutionConstAccessor> acc = accessorForExistingTable();
       CPPUNIT_ASSERT(acc);
       // only 6 antennas, 3 beams and 8 channels are defined
       acc->bandpass(JonesIndex(0u,0u),8);
   }
   
   void testUndefinedSolution() {
       const boost::shared_ptr<ICalSolutionConstSource> css = roSource();
       CPPUNIT_ASSERT(css);
       const long id = css->solutionID(0.5);
       CPPUNIT_ASSERT_EQUAL(0l, id);
       const boost::shared_ptr<ICalSolutionConstAccessor> acc = css->roSolution(id);
       CPPUNIT_ASSERT(acc);
       try {
          // the following should be successful because the first solution in the table 
          // was the gain solution
          const JonesJTerm gain = acc->gain(JonesIndex(0u,0u));
          testComplex(casa::Complex(1.,-1.), gain.g1());
          testComplex(casa::Complex(-1.,1.), gain.g2());
          CPPUNIT_ASSERT(gain.g1IsValid());
          CPPUNIT_ASSERT(gain.g2IsValid());                               
       } 
       catch (const AskapError &) {
          // this shouldn't have happened
          CPPUNIT_ASSERT(false);
       }
       // the following should cause an exception because no leakage is defined at or before row 0.
       acc->leakage(JonesIndex(0u,0u));
   }
   
};

} // namespace accessors

} // namespace askap

