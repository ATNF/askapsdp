/// @file
///
/// Unit test for the memory-based implementation of the interface to access
/// calibration solutions. It is also used in the table-based implementation. 
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

// own includes
#include <calibaccess/JonesIndex.h>
#include <calibaccess/MemCalSolutionAccessor.h>
#include <calibaccess/ICalSolutionFiller.h>
#include <askap/AskapUtil.h>


// boost includes
#include <boost/shared_ptr.hpp>


namespace askap {

namespace accessors {

class MemCalSolutionAccessorTest : public CppUnit::TestFixture,
                                   virtual public ICalSolutionFiller
{
   CPPUNIT_TEST_SUITE(MemCalSolutionAccessorTest);
   CPPUNIT_TEST(testRead);
   CPPUNIT_TEST_SUITE_END();
protected:
   static void fillCube(casa::Cube<casa::Complex> &cube) {
      for (casa::uInt row=0; row<cube.nrow(); ++row) {
           for (casa::uInt column=0; column<cube.ncolumn(); ++column) {
                for (casa::uInt plane=0; plane<cube.nplane(); ++plane) {
                     const float scale = (row / 2 + 1) * (row % 2 == 0 ? 1. : -1.);
                     cube(row,column,plane) = casa::Complex(scale*(float(column)/100. + float(plane)/10.),
                                                  -scale*(float(column)/100. + float(plane)/10.));
                }
           }
      }
   }
   
   void testValue(const casa::Complex &val, const JonesIndex &index, const casa::uInt row) const {
      const casa::uInt ant = casa::uInt(index.antenna());
      const casa::uInt beam = casa::uInt(index.beam());
      CPPUNIT_ASSERT(ant < itsNAnt);
      CPPUNIT_ASSERT(beam < itsNBeam);
      const float scale = (row / 2 + 1) * (row % 2 == 0 ? 1. : -1.);
      const casa::Complex expected(scale*(float(ant)/100. + float(beam)/10.),
                                                  -scale*(float(ant)/100. + float(beam)/10.));      
      CPPUNIT_ASSERT_DOUBLES_EQUAL(real(expected), real(val), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(imag(expected), imag(val), 1e-6);      
   }
   
public:
   void setUp() {
      itsNAnt = 36;
      itsNBeam = 30;
      itsNChan = 256;
      // flags showing  that write operation has taken place
      itsGainsWritten  = false;
      itsLeakagesWritten  = false;
      itsBandpassesWritten  = false;      
      // flags showing that read operation has taken place
      itsGainsRead  = false;
      itsLeakagesRead  = false;
      itsBandpassesRead  = false;            
   }

  // methods of the solution filler
  /// @brief gains filler  
  /// @details
  /// @param[in] gains pair of cubes with gains and validity flags (to be resised to 2 x nAnt x nBeam)
  virtual void fillGains(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &gains) const {
     gains.first.resize(2, itsNAnt, itsNBeam);
     gains.second.resize(2, itsNAnt, itsNBeam);
     gains.second.set(true);     
     fillCube(gains.first);
     itsGainsRead = true;
  }
  
  /// @brief leakage filler  
  /// @details
  /// @param[in] leakages pair of cubes with leakages and validity flags (to be resised to 2 x nAnt x nBeam)
  virtual void fillLeakages(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &leakages) const {
     leakages.first.resize(2, itsNAnt, itsNBeam);
     leakages.second.resize(2, itsNAnt, itsNBeam);
     leakages.second.set(true);     
     fillCube(leakages.first);
     itsLeakagesRead = true;
  }

  /// @brief bandpass filler  
  /// @details
  /// @param[in] bp pair of cubes with bandpasses and validity flags (to be resised to (2*nChan) x nAnt x nBeam)
  virtual void fillBandpasses(std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &bp) const {
     bp.first.resize(2*itsNChan, itsNAnt, itsNBeam);
     bp.second.resize(2*itsNChan, itsNAnt, itsNBeam);
     bp.second.set(true);     
     fillCube(bp.first);  
     itsBandpassesRead = true;
  }
  
  /// @brief gains writer
  /// @details
  /// @param[in] gains pair of cubes with gains and validity flags (should be 2 x nAnt x nBeam)
  virtual void writeGains(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &gains) const  {
     CPPUNIT_ASSERT(gains.first.shape() == gains.second.shape());
     CPPUNIT_ASSERT_EQUAL(2u,gains.first.nrow());
     CPPUNIT_ASSERT_EQUAL(itsNAnt,gains.first.ncolumn());
     CPPUNIT_ASSERT_EQUAL(itsNBeam,gains.first.nplane());      
     itsGainsWritten = true;
  }
  
  /// @brief leakage writer  
  /// @details
  /// @param[in] leakages pair of cubes with leakages and validity flags (should be 2 x nAnt x nBeam)
  virtual void writeLeakages(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &leakages) const {
     CPPUNIT_ASSERT(leakages.first.shape() == leakages.second.shape());
     CPPUNIT_ASSERT_EQUAL(2u,leakages.first.nrow());
     CPPUNIT_ASSERT_EQUAL(itsNAnt,leakages.first.ncolumn());
     CPPUNIT_ASSERT_EQUAL(itsNBeam,leakages.first.nplane());      
     itsLeakagesWritten = true;  
  }

  /// @brief bandpass writer  
  /// @details
  /// @param[in] bp pair of cubes with bandpasses and validity flags (should be (2*nChan) x nAnt x nBeam)
  virtual void writeBandpasses(const std::pair<casa::Cube<casa::Complex>, casa::Cube<casa::Bool> > &bp) const {
     CPPUNIT_ASSERT(bp.first.shape() == bp.second.shape());
     CPPUNIT_ASSERT_EQUAL(2*itsNChan,bp.first.nrow());
     CPPUNIT_ASSERT_EQUAL(itsNAnt,bp.first.ncolumn());
     CPPUNIT_ASSERT_EQUAL(itsNBeam,bp.first.nplane());      
     itsBandpassesWritten = true;    
  }
   
  // test methods
  void testRead() {
     boost::shared_ptr<ICalSolutionFiller> csf(this, utility::NullDeleter());
     boost::shared_ptr<MemCalSolutionAccessor> itsAccessor(new MemCalSolutionAccessor(csf,true));
     CPPUNIT_ASSERT(itsAccessor);
     CPPUNIT_ASSERT(!itsGainsRead);
     CPPUNIT_ASSERT(!itsLeakagesRead);
     CPPUNIT_ASSERT(!itsBandpassesRead);
     CPPUNIT_ASSERT(!itsGainsWritten);
     CPPUNIT_ASSERT(!itsLeakagesWritten);
     CPPUNIT_ASSERT(!itsBandpassesWritten);
     for (casa::uInt ant = 0; ant<itsNAnt; ++ant) {
          for (casa::uInt beam = 0; beam<itsNBeam; ++beam) {
               const JonesIndex index(ant,beam);
               const JonesJTerm gain = itsAccessor->gain(index);
               CPPUNIT_ASSERT(gain.g1IsValid());
               CPPUNIT_ASSERT(gain.g2IsValid());               
               testValue(gain.g1(),index,0);
               testValue(gain.g2(),index,1);               
          }
     }
     CPPUNIT_ASSERT(itsGainsRead);
     CPPUNIT_ASSERT(!itsLeakagesRead);
     CPPUNIT_ASSERT(!itsBandpassesRead);
     CPPUNIT_ASSERT(!itsGainsWritten);
     CPPUNIT_ASSERT(!itsLeakagesWritten);
     CPPUNIT_ASSERT(!itsBandpassesWritten);
     
  }
private:
  casa::uInt itsNAnt;
  casa::uInt itsNBeam;
  casa::uInt itsNChan; 
  mutable bool itsGainsWritten;
  mutable bool itsLeakagesWritten;
  mutable bool itsBandpassesWritten;
  mutable bool itsGainsRead;
  mutable bool itsLeakagesRead;
  mutable bool itsBandpassesRead;    
};

} // namespace accessors

} // namespace askap


