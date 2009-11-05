/// @copyright (c) 2007 CSIRO
/// 
/// Tests gathered in this directory are testing 3rd party libraries we use in synthesis, like casacore,
/// rather than the actual synthesis code itself. It is largely to ensure that our interpretation of e.g.
/// image projection and coordinate conversion interfaces is correct and does not change if casacore, etc evolves
/// (i.e. because existing documentation aghs gaps)
/// 
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

// ASKAPsoft includes
#include <AskapTestRunner.h>

// Test includes
#include <CasaProjectionTest.h>

int main(int argc, char *argv[])
{
    askapdev::testutils::AskapTestRunner runner(argv[0]);
    runner.addTest( askap::synthesis::CasaProjectionTest::suite());
    bool wasSucessful = runner.run();

    return wasSucessful ? 0 : 1;
}
