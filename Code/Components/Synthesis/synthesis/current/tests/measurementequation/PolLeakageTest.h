/// @file
/// 
/// @brief Unit tests for polarisation leakage calibration.
/// @details The tests gathered in this file predict visibility data
/// with some calibration errors and then solve for them.
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

#ifndef POL_LEAKAGE_TEST_H
#define POL_LEAKAGE_TEST_H

#include <measurementequation/ComponentEquation.h>
#include <measurementequation/CalibrationME.h>

#include <fitting/LinearSolver.h>
#include <dataaccess/DataIteratorStub.h>
#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <boost/shared_ptr.hpp>


namespace askap
{
  namespace synthesis
  {

    class PolLeakageTest : public CppUnit::TestFixture
    {
      CPPUNIT_TEST_SUITE(PolLeakageTest);
      CPPUNIT_TEST(testSolve);
      CPPUNIT_TEST_SUITE_END();
     public:
      void testSolve() {} 
     private: 
    };
  } // namespace synthesis

} // namespace askap


#endif // #ifndef POL_LEAKAGE_TEST_H



