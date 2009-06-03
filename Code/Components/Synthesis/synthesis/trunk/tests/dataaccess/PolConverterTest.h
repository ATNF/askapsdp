/// @file
/// $brief Tests of the polarisation frame converter
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
/// 
#ifndef POL_CONVERTER_TEST_H
#define POL_CONVERTER_TEST_H

// cppunit includes
#include <cppunit/extensions/HelperMacros.h>
// own includes
#include <dataaccess/PolConverter.h>

namespace askap {

namespace synthesis {

class PolConverterTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PolConverterTest);
  CPPUNIT_TEST(dimensionTest);
  CPPUNIT_TEST_SUITE_END();
public:
  void dimensionTest() {
     casa::Vector<casa::Stokes::StokesTypes> in(4);
     in[0] = casa::Stokes::XX;
     in[1] = casa::Stokes::XY;
     in[2] = casa::Stokes::YX;
     in[3] = casa::Stokes::YY;
     casa::Vector<casa::Stokes::StokesTypes> out(2);
     out[0] = casa::Stokes::I;
     out[1] = casa::Stokes::Q;
     
     PolConverter pc(in,out);
     casa::Vector<casa::Complex> inVec(in.nelements(), casa::Complex(0,-1.));
     casa::Vector<casa::Complex> outVec = pc(inVec);
     CPPUNIT_ASSERT(outVec.nelements() == out.nelements());
  }
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef POL_CONVERTER_TEST_H

