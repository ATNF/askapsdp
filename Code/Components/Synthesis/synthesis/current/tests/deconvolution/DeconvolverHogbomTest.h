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

#include <deconvolution/DeconvolverHogbom.h>
#include <cppunit/extensions/HelperMacros.h>

#include <casa/BasicSL/Complex.h>

#include <boost/shared_ptr.hpp>

using namespace casa;

namespace askap {

namespace synthesis {

class DeconvolverHogbomTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DeconvolverHogbomTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testOneIteration);
  CPPUNIT_TEST(testDeconvolve);
  CPPUNIT_TEST(testDeconvolveZero);
  CPPUNIT_TEST_EXCEPTION(testWrongShape, AskapError);
  CPPUNIT_TEST_SUITE_END();
public:
   
  void setUp() {
    IPosition dimensions(2,100,100);
    itsDirty.reset(new Array<Float>(dimensions));
    itsPsf.reset(new Array<Float>(dimensions));
    itsDB = DeconvolverHogbom<Float,Complex>::ShPtr(new DeconvolverHogbom<Float, Complex>(*itsDirty, *itsPsf));
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
    itsMask.reset(new Array<Float>(dimensions));
    itsMask->set(1.0);
    itsWeight.reset(new Array<Float>(dimensions));
    itsWeight->set(10.0);
    itsDB->setMask(*itsMask);
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
    Array<Float> newDirty(IPosition(2,100,100));
    itsDB->updateDirty(newDirty);
  }
  void testWrongShape() {
    Array<Float> newDirty(IPosition(2,200,200));
    itsDB->updateDirty(newDirty);
  }
  void testOneIteration() {
    itsDB->state()->setCurrentIter(0);
    itsDB->control()->setTargetIter(10);
    itsDB->control()->setGain(1.0);
    itsDB->control()->setTargetObjectiveFunction(0.001);
    itsDB->dirty().set(1.0);
    itsDB->initialise();
    CPPUNIT_ASSERT(itsDB->oneIteration());
    CPPUNIT_ASSERT(itsDB->control()->terminationCause()==DeconvolverControl<Float>::NOTTERMINATED);
  }
  void testDeconvolveZero() {
    itsDB->state()->setCurrentIter(0);
    itsDB->control()->setTargetIter(10);
    itsDB->control()->setGain(1.0);
    itsDB->control()->setTargetObjectiveFunction(0.001);
    itsDB->dirty().set(0.0);
    CPPUNIT_ASSERT(itsDB->deconvolve());
    CPPUNIT_ASSERT(itsDB->control()->terminationCause()==DeconvolverControl<Float>::CONVERGED);
  }
  void testDeconvolve() {
    itsDB->state()->setCurrentIter(0);
    itsDB->control()->setTargetIter(10);
    itsDB->control()->setGain(1.0);
    itsDB->control()->setTargetObjectiveFunction(0.001);
    itsDB->dirty().set(1.0);
    CPPUNIT_ASSERT(itsDB->deconvolve());
    CPPUNIT_ASSERT(itsDB->control()->terminationCause()==DeconvolverControl<Float>::EXCEEDEDITERATIONS);
  }
   
private:

  boost::shared_ptr< Array<Float> > itsDirty;
  boost::shared_ptr< Array<Float> > itsPsf;
  boost::shared_ptr< Array<Float> > itsMask;
  boost::shared_ptr< Array<Float> > itsWeight;

   /// @brief DeconvolutionHogbom class
  boost::shared_ptr<DeconvolverHogbom<Float, Complex> > itsDB;
};
    
} // namespace synthesis

} // namespace askap

