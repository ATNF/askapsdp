/// @file
///
/// Unit test for the deconvolution base class
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

#include <deconvolution/DeconvolverBase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <casa/BasicSL/Complex.h>

#include <boost/shared_ptr.hpp>

using namespace casa;

namespace askap {

namespace synthesis {

class DeconvolverBaseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DeconvolverBaseTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreatePlugins);
  CPPUNIT_TEST_EXCEPTION(testWrongShape, casa::ArrayShapeError);
  CPPUNIT_TEST_EXCEPTION(testMoreDim, AskapError);
  CPPUNIT_TEST_SUITE_END();
public:
   
  void testCreate() {
    Array<Float> dirty(IPosition(2,100,100));
    Array<Float> psf(IPosition(2,100,100));
    itsDB = DeconvolverBase<Float,Complex>::ShPtr(new DeconvolverBase<Float, Complex>(dirty, psf));
    CPPUNIT_ASSERT(itsDB);
    CPPUNIT_ASSERT(itsDB->control());
    CPPUNIT_ASSERT(itsDB->monitor());
    CPPUNIT_ASSERT(itsDB->state());
    Array<Float> newDirty(IPosition(2,100,100));
    itsDB->updateDirty(newDirty);
  }
  void testCreatePlugins() {
    Array<Float> dirty(IPosition(2,100,100));
    Array<Float> psf(IPosition(2,100,100));
    itsDB = DeconvolverBase<Float,Complex>::ShPtr(new DeconvolverBase<Float, Complex>(dirty, psf));
    CPPUNIT_ASSERT(itsDB);
    boost::shared_ptr<DeconvolverControl<Float> > DC(new DeconvolverControl<Float>::DeconvolverControl());
    CPPUNIT_ASSERT(itsDB->setControl(DC));
  }
  void testWrongShape() {
    Array<Float> dirty(IPosition(2,100,100));
    Array<Float> psf(IPosition(2,100,100));
    itsDB = DeconvolverBase<Float,Complex>::ShPtr(new DeconvolverBase<Float, Complex>(dirty, psf));
    CPPUNIT_ASSERT(itsDB);
    Array<Float> newDirty(IPosition(2,200,200));
    itsDB->updateDirty(newDirty);
  }
   
  void testMoreDim() {
    Array<Float> dirty(IPosition(4,100,100,0,0));
    Array<Float> psf(IPosition(4,100,100,0,0));
    itsDB = DeconvolverBase<Float,Complex>::ShPtr(new DeconvolverBase<Float, Complex>(dirty, psf));
    CPPUNIT_ASSERT(itsDB);
    Array<Float> newDirty(IPosition(2,200,200));
    itsDB->updateDirty(newDirty);
  }
   
private:
   /// @brief DeconvolutionBase class
  boost::shared_ptr<DeconvolverBase<Float, Complex> > itsDB;
};
    
} // namespace synthesis

} // namespace askap

