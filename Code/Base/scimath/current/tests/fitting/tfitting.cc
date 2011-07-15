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

// ASKAPsoft includes
#include <AskapTestRunner.h>

// Test includes
#include <ParamsTest.h>
#include <ParamsTableTest.h>
#include <DesignMatrixTest.h>
#include <ImagingNormalEquationsTest.h>
#include <GenericNormalEquationsTest.h>
#include <NormalEquationsStubTest.h>
#include <PolynomialEquationTest.h>
#include <GeneralFittingTest.h>
#include <ComplexDiffTest.h>
#include <ComplexDiffMatrixTest.h>
#include <AxesTest.h>
#include <PolXProductsTest.h>

int main(int argc, char *argv[])
{
    askapdev::testutils::AskapTestRunner runner(argv[0]);

    runner.addTest(askap::scimath::AxesTest::suite());
    runner.addTest(askap::scimath::ParamsTest::suite());
    runner.addTest(askap::scimath::ParamsTableTest::suite());
    runner.addTest(askap::scimath::DesignMatrixTest::suite());
    runner.addTest(askap::scimath::GenericNormalEquationsTest::suite());
    runner.addTest(askap::scimath::ImagingNormalEquationsTest::suite());
    runner.addTest(askap::scimath::NormalEquationsStubTest::suite());
    runner.addTest(askap::scimath::PolynomialEquationTest::suite());
    runner.addTest(askap::scimath::GeneralFittingTest::suite());
    runner.addTest(askap::scimath::ComplexDiffTest::suite());
    runner.addTest(askap::scimath::ComplexDiffMatrixTest::suite());
    runner.addTest(askap::scimath::PolXProductsTest::suite());

    bool wasSucessful = runner.run();

    return wasSucessful ? 0 : 1;
}

