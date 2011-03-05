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

#include <deconvolution/DeconvolverFista.h>
#include <cppunit/extensions/HelperMacros.h>

#include <casa/BasicSL/Complex.h>

#include <boost/shared_ptr.hpp>

using namespace casa;

namespace askap {

namespace synthesis {

class DeconvolverFistaTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DeconvolverFistaTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testDeconvolve);
  CPPUNIT_TEST_EXCEPTION(testWrongShape, casa::ArrayShapeError);
  CPPUNIT_TEST_SUITE_END();
public:
   
  void setUp() {
    itsDimensions=IPosition(4,100,100,1,1);
    itsDirty.reset(new Array<Float>(itsDimensions));
    itsDirty->set(0.0);
    itsPsf.reset(new Array<Float>(itsDimensions));
    itsPsf->set(0.0);
    (*itsPsf)(IPosition(4,50,50,0,0))=1.0;
    itsDB = DeconvolverFista<Float,Complex>::ShPtr(new DeconvolverFista<Float, Complex>(*itsDirty, *itsPsf));
    CPPUNIT_ASSERT(itsDB);
    CPPUNIT_ASSERT(itsDB->control());
    CPPUNIT_ASSERT(itsDB->monitor());
    CPPUNIT_ASSERT(itsDB->state());
    boost::shared_ptr<DeconvolverControl<Float> > DC(new DeconvolverControl<Float>::DeconvolverControl());
    CPPUNIT_ASSERT(itsDB->setControl(DC));
    boost::shared_ptr<DeconvolverMonitor<Float> > DM(new DeconvolverMonitor<Float>::DeconvolverMonitor());
    CPPUNIT_ASSERT(itsDB->setMonitor(DM));
    boost::shared_ptr<DeconvolverState<Float> > DS(new DeconvolverState<Float>::DeconvolverState());
    CPPUNIT_ASSERT(itsDB->setControl(DC));
    itsWeight.reset(new Array<Float>(itsDimensions));
    itsWeight->set(10.0);
    itsDB->setWeight(*itsWeight);
  }

  void tearDown() {
      // Ensure arrays are destroyed last
      itsDB.reset();
      itsWeight.reset();
      itsMask.reset();
      itsPsf.reset();
      itsDirty.reset();
  }

  void testCreate() {
    Array<Float> newDirty(itsDimensions);
    itsDB->updateDirty(newDirty);
  }
  void testWrongShape() {
    Array<Float> newDirty(IPosition(2,200,200));
    itsDB->updateDirty(newDirty);
  }
  void testDeconvolve() {
    itsDB->state()->setCurrentIter(0);
    itsDB->control()->setTargetIter(10);
    itsDB->control()->setGain(1.0);
    itsDB->control()->setTargetObjectiveFunction(0.000); 
    itsDB->control()->setLambda(0.00001);
    itsDB->dirty().set(0.0);
    itsDB->dirty()(IPosition(4,30,20,0,0))=1.0;
    CPPUNIT_ASSERT(itsDB->deconvolve());
  }

private:

  IPosition itsDimensions;

  boost::shared_ptr< Array<Float> > itsDirty;
  boost::shared_ptr< Array<Float> > itsPsf;
  boost::shared_ptr< Array<Float> > itsMask;
  boost::shared_ptr< Array<Float> > itsWeight;

   /// @brief DeconvolutionFista class
  boost::shared_ptr<DeconvolverFista<Float, Complex> > itsDB;
};
    
} // namespace synthesis

} // namespace askap

