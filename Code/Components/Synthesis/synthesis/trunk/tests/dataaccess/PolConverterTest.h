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
#include <askap/AskapError.h>

namespace askap {

namespace synthesis {

class PolConverterTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PolConverterTest);
  CPPUNIT_TEST(dimensionTest);
  CPPUNIT_TEST_EXCEPTION(dimensionExceptionTest, askap::CheckError);
  CPPUNIT_TEST(stokesEnumTest);
  CPPUNIT_TEST(stringConversionTest);
  CPPUNIT_TEST(linear2stokesTest);
  CPPUNIT_TEST(circular2stokesTest);
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
     CPPUNIT_ASSERT(abs(outVec[0]-casa::Complex(0.,-2.))<1e-5);
     CPPUNIT_ASSERT(abs(outVec[1]-casa::Complex(0.,0.))<1e-5);
     
     // ignore missing polarisations in pc2
     PolConverter pc2(out,in,false);
     casa::Vector<casa::Complex> inVec2(out.nelements(), casa::Complex(0,-1.));
     casa::Vector<casa::Complex> outVec2 = pc2(inVec2);
     CPPUNIT_ASSERT(outVec2.nelements() == in.nelements());
     CPPUNIT_ASSERT(abs(outVec2[0]-casa::Complex(0.,-1.))<1e-5);
     CPPUNIT_ASSERT(abs(outVec2[1]-casa::Complex(0.,0.))<1e-5);
     CPPUNIT_ASSERT(abs(outVec2[2]-casa::Complex(0.,0.))<1e-5);
     CPPUNIT_ASSERT(abs(outVec2[3]-casa::Complex(0.,0.))<1e-5);     
  }
  
  void dimensionExceptionTest() {
     casa::Vector<casa::Stokes::StokesTypes> in(2);
     in[0] = casa::Stokes::I;
     in[1] = casa::Stokes::Q;

     casa::Vector<casa::Stokes::StokesTypes> out(4);
     out[0] = casa::Stokes::XX;
     out[1] = casa::Stokes::XY;
     out[2] = casa::Stokes::YX;
     out[3] = casa::Stokes::YY;
     
     // don't ignore missing polarisations here (default third argument), this should cause an
     // exception in the constructor     
     PolConverter pc(in,out);
     casa::Vector<casa::Complex> inVec(in.nelements(), casa::Complex(0,-1.));
     pc(inVec);  
  }
  
  void linear2stokesTest() {
     casa::Vector<casa::Stokes::StokesTypes> in(4);
     in[0] = casa::Stokes::XX;
     in[1] = casa::Stokes::XY;
     in[2] = casa::Stokes::YX;
     in[3] = casa::Stokes::YY;
     casa::Vector<casa::Stokes::StokesTypes> out(4);
     out[0] = casa::Stokes::I;
     out[1] = casa::Stokes::Q;
     out[2] = casa::Stokes::U;
     out[3] = casa::Stokes::V;
     
     PolConverter pc(in,out);
     casa::Vector<casa::Complex> inVec(in.nelements());
     inVec[0]=casa::Complex(0.1,0.2);
     inVec[1]=casa::Complex(0.3,0.4);
     inVec[2]=casa::Complex(0.5,0.6);
     inVec[3]=casa::Complex(0.7,0.8);     
     casa::Vector<casa::Complex> outVec = pc(inVec);
     CPPUNIT_ASSERT(outVec.nelements() == out.nelements());
     CPPUNIT_ASSERT(abs(outVec[0]-casa::Complex(0.8,1))<1e-5);
     CPPUNIT_ASSERT(abs(outVec[1]-casa::Complex(-0.6,-0.6))<1e-5);
     CPPUNIT_ASSERT(abs(outVec[2]-casa::Complex(0.8,1.0))<1e-5);
     CPPUNIT_ASSERT(abs(outVec[3]-casa::Complex(-0.2,0.2))<1e-5);
     
     PolConverter pcReverse(out,in);
     casa::Vector<casa::Complex> newInVec = pcReverse(outVec);
     CPPUNIT_ASSERT(newInVec.nelements() == inVec.nelements());
     for (size_t pol = 0; pol<inVec.nelements(); ++pol) {
          CPPUNIT_ASSERT(abs(inVec[pol] - newInVec[pol])<1e-5);
     }     
  }

  void circular2stokesTest() {
     casa::Vector<casa::Stokes::StokesTypes> in(4);
     in[0] = casa::Stokes::RR;
     in[1] = casa::Stokes::RL;
     in[2] = casa::Stokes::LR;
     in[3] = casa::Stokes::LL;
     casa::Vector<casa::Stokes::StokesTypes> out(4);
     out[0] = casa::Stokes::I;
     out[1] = casa::Stokes::Q;
     out[2] = casa::Stokes::U;
     out[3] = casa::Stokes::V;
     
     PolConverter pc(in,out);
     casa::Vector<casa::Complex> inVec(in.nelements());
     inVec[0]=casa::Complex(0.1,0.2);
     inVec[1]=casa::Complex(0.3,0.4);
     inVec[2]=casa::Complex(0.5,0.6);
     inVec[3]=casa::Complex(0.7,0.8);     
     casa::Vector<casa::Complex> outVec = pc(inVec);
     CPPUNIT_ASSERT(outVec.nelements() == out.nelements());
     CPPUNIT_ASSERT(abs(outVec[0]-casa::Complex(0.8,1))<1e-5);
     CPPUNIT_ASSERT(abs(outVec[1]-casa::Complex(-0.2,0.2))<1e-5);
     CPPUNIT_ASSERT(abs(outVec[2]-casa::Complex(-0.6,-0.6))<1e-5);
     CPPUNIT_ASSERT(abs(outVec[3]-casa::Complex(0.8,1.0))<1e-5);

     PolConverter pcReverse(out,in);
     casa::Vector<casa::Complex> newInVec = pcReverse(outVec);
     CPPUNIT_ASSERT(newInVec.nelements() == inVec.nelements());
     for (size_t pol = 0; pol<inVec.nelements(); ++pol) {
          CPPUNIT_ASSERT(abs(inVec[pol] - newInVec[pol])<1e-5);
     }     
  }
  
  void stokesEnumTest() {
     // our code relies on a particular order of the stokes parameters in the enum defined in casacore.
     // The following code tests that enums components corresponding to the same polarisation
     // frame are following each other and the order is preserved.
     
     // I,Q,U,V
     CPPUNIT_ASSERT(int(casa::Stokes::Q)-int(casa::Stokes::I) == 1);
     CPPUNIT_ASSERT(int(casa::Stokes::U)-int(casa::Stokes::I) == 2);
     CPPUNIT_ASSERT(int(casa::Stokes::V)-int(casa::Stokes::I) == 3);
     
     // XX,XY,YX,YY
     CPPUNIT_ASSERT(int(casa::Stokes::XY)-int(casa::Stokes::XX) == 1);
     CPPUNIT_ASSERT(int(casa::Stokes::YX)-int(casa::Stokes::XX) == 2);
     CPPUNIT_ASSERT(int(casa::Stokes::YY)-int(casa::Stokes::XX) == 3);
     
     // RR,RL,LR,LL
     CPPUNIT_ASSERT(int(casa::Stokes::RL)-int(casa::Stokes::RR) == 1);
     CPPUNIT_ASSERT(int(casa::Stokes::LR)-int(casa::Stokes::RR) == 2);
     CPPUNIT_ASSERT(int(casa::Stokes::LL)-int(casa::Stokes::RR) == 3);
     
     // mixed products
     CPPUNIT_ASSERT(int(casa::Stokes::RY)-int(casa::Stokes::RX) == 1);
     CPPUNIT_ASSERT(int(casa::Stokes::LX)-int(casa::Stokes::RX) == 2);
     CPPUNIT_ASSERT(int(casa::Stokes::LY)-int(casa::Stokes::RX) == 3);
     CPPUNIT_ASSERT(int(casa::Stokes::XR)-int(casa::Stokes::RX) == 4);
     CPPUNIT_ASSERT(int(casa::Stokes::XL)-int(casa::Stokes::RX) == 5);
     CPPUNIT_ASSERT(int(casa::Stokes::YR)-int(casa::Stokes::RX) == 6);
     CPPUNIT_ASSERT(int(casa::Stokes::YL)-int(casa::Stokes::RX) == 7);
     
  }  
  void stringConversionTest() {
     CPPUNIT_ASSERT(PolConverter::equal(PolConverter::fromString("xx,yy,xy,yx"),
                    PolConverter::fromString("xxyyxyyx")));
     CPPUNIT_ASSERT(PolConverter::equal(PolConverter::fromString("xyi,qu"),
                    PolConverter::fromString("xy i q u")));
     casa::Vector<casa::Stokes::StokesTypes> frame = PolConverter::fromString("xy i q RR");
     CPPUNIT_ASSERT(frame.nelements() == 4);
     CPPUNIT_ASSERT(frame[0] == casa::Stokes::XY);
     CPPUNIT_ASSERT(frame[1] == casa::Stokes::I);
     CPPUNIT_ASSERT(frame[2] == casa::Stokes::Q);
     CPPUNIT_ASSERT(frame[3] == casa::Stokes::RR);
     std::vector<std::string> frameStr = PolConverter::toString(frame);
     CPPUNIT_ASSERT(frameStr.size() == 4);
     CPPUNIT_ASSERT(frameStr[0] == "XY");
     CPPUNIT_ASSERT(frameStr[1] == "I");
     CPPUNIT_ASSERT(frameStr[2] == "Q");
     CPPUNIT_ASSERT(frameStr[3] == "RR");     
  }
  
};

} // namespace synthesis

} // namespace askap

#endif // #ifndef POL_CONVERTER_TEST_H

