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

#include <cppunit/ui/text/TestRunner.h>

#include <ComponentEquationTest.h>
#include <VectorOperationsTest.h>
#include <ImageDFTEquationTest.h>
#include <ImageFFTEquationTest.h>
#include <CalibrationMETest.h>
#include <PreconditionerTests.h>
#include <SynthesisParamsHelperTest.h>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
/*
  runner.addTest( askap::synthesis::VectorOperationsTest::suite() );
  runner.addTest( askap::synthesis::ComponentEquationTest::suite() );
  runner.addTest( askap::synthesis::CalibrationMETest::suite() );
  //runner.addTest( askap::synthesis::ImageDFTEquationTest::suite() );
  runner.addTest( askap::synthesis::ImageFFTEquationTest::suite() );
  runner.addTest( askap::synthesis::PreconditionerTests::suite() );
*/
  runner.addTest( askap::synthesis::SynthesisParamsHelperTest::suite() );
  runner.run();
  return 0;
}
