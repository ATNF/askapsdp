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

#include <measurementequation/ImageFFTEquation.h>
//#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/AWProjectVisGridder.h>
#include <gridding/DiskIllumination.h>
#include <measurementequation/ImageSolver.h>
#include <measurementequation/ImageMultiScaleSolver.h>
#include <dataaccess/DataIteratorStub.h>
#include <fitting/ParamsCasaTable.h>

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/BasicSL/Constants.h>

#include <askap/AskapError.h>

#include <cppunit/extensions/HelperMacros.h>

#include <stdexcept>

#include <boost/shared_ptr.hpp>

using namespace askap;
using namespace askap::scimath;

namespace askap
{
  namespace synthesis
  {

    class ImageFFTEquationTest : public CppUnit::TestFixture
    {

      CPPUNIT_TEST_SUITE(ImageFFTEquationTest);
      CPPUNIT_TEST(testPredict);
      CPPUNIT_TEST(testSolveSphFun);
      CPPUNIT_TEST(testSolveAntIllum);
      CPPUNIT_TEST_EXCEPTION(testFixed, CheckError);
      CPPUNIT_TEST_SUITE_END();

  private:
      boost::shared_ptr<ImageFFTEquation> p1, p2;
      boost::shared_ptr<Params> params1, params2;
      IDataSharedIter idi;
      uint npix;

  public:
      void setUp()
      {
        idi = IDataSharedIter(new DataIteratorStub(1));

        npix=1024;
        Axes imageAxes;
        double arcsec=casa::C::pi/(3600.0*180.0);
        double cell=8.0*arcsec;
        imageAxes.add("RA", -double(npix)*cell/2.0, double(npix)*cell/2.0);
        imageAxes.add("DEC", -double(npix)*cell/2.0, double(npix)*cell/2.0);
        imageAxes.addStokesAxis(casa::Vector<casa::Stokes::StokesTypes>(1,casa::Stokes::I));
        imageAxes.add("FREQUENCY",1.4e9,1.4e9);

        params1.reset(new Params);
        casa::Array<double> imagePixels1(casa::IPosition(4, npix, npix, 1, 1));
        imagePixels1.set(0.0);
        imagePixels1(casa::IPosition(4, npix/2, npix/2, 0, 0))=1.0;
        imagePixels1(casa::IPosition(4, 3*npix/8, 7*npix/16, 0, 0))=0.7;
        params1->add("image.i.cena", imagePixels1, imageAxes);

        p1.reset(new ImageFFTEquation(*params1, idi));

        params2.reset(new Params);
        casa::Array<double> imagePixels2(casa::IPosition(4, npix, npix, 1, 1));
        imagePixels2.set(0.0);
        imagePixels2(casa::IPosition(4, npix/2, npix/2, 0, 0))=0.9;
        imagePixels2(casa::IPosition(4, 3*npix/8, 7*npix/16, 0, 0))=0.75;
        params2->add("image.i.cena", imagePixels2, imageAxes);
        p2.reset(new ImageFFTEquation(*params2, idi));

      }

      void tearDown()
      {
      }

      void testPredict()
      {
        //        {
        //          ParamsCasaTable pt("ImageFFTEquationTest_original.tab", false);
        //          pt.setParameters(*params1);
        //        }
        p1->predict();
      }

      void testSolveSphFun()
      {
        // Predict with the "perfect" parameters"
        p1->predict();
        // perform a given number of major cycles
        const size_t nMajCycles = 1;
        casa::Array<double> improved; // buffer for the result
        for (size_t cycle = 0; cycle<nMajCycles; ++cycle) {
             // Calculate gradients using "imperfect" parameters" 
             ImagingNormalEquations ne(*params2);
             p2->calcEquations(ne);
             Quality q;
             ImageMultiScaleSolver solver1(*params2);
             solver1.setAlgorithm("Hogbom");
             solver1.addNormalEquations(ne);
             solver1.solveNormalEquations(q);
             improved=solver1.parameters().value("image.i.cena");
        }
        //        {
        //          ParamsCasaTable pt("ImageFFTEquationTest_SphFun.tab", false);
        //          pt.setParameters(solver1.parameters());
        //        }
        // This only works for the pixels with emission but it's a good test nevertheless
        CPPUNIT_ASSERT(abs(improved(casa::IPosition(4, npix/2, npix/2, 0, 0))
            -1.0)<0.003);
        CPPUNIT_ASSERT(abs(improved(casa::IPosition(4, 3*npix/8, 7*npix/16, 0,
            0))-0.700)<0.003);
      }

      void testSolveAntIllum()
      {
        // Predict with the "perfect" parameters"
        boost::shared_ptr<IBasicIllumination> illum(new DiskIllumination(12.0, 1.0));
        
        IVisGridder::ShPtr gridder=IVisGridder::ShPtr(new AWProjectVisGridder(illum,
									      8000, 9, 1e-3, 8, 512, 0));
        p1.reset(new ImageFFTEquation(*params1, idi, gridder));
        p2.reset(new ImageFFTEquation(*params2, idi, gridder));
        p1->predict();
        // Calculate gradients using "imperfect" parameters"
        ImagingNormalEquations ne(*params2);
        p2->calcEquations(ne);
        Quality q;
        ImageSolver solver1(*params2);
        solver1.addNormalEquations(ne);
        solver1.solveNormalEquations(q);
        const casa::Array<double> improved=solver1.parameters().value("image.i.cena");
        //        {
        //          ParamsCasaTable pt("ImageFFTEquationTest_AntIllum.tab", false);
        //          pt.setParameters(solver1.parameters());
        //        }
        // This only works for the pixels with emission but it's a good test nevertheless
        CPPUNIT_ASSERT(abs(improved(casa::IPosition(4, npix/2, npix/2, 0, 0))
            -1.0)<0.005);
        CPPUNIT_ASSERT(abs(improved(casa::IPosition(4, 3*npix/8, 7*npix/16, 0,
            0))-0.700)<0.005);
      }

      void testFixed()
      {
        ImagingNormalEquations ne(*params1);
        p1->predict();
        p2->calcEquations(ne);
        Quality q;
        params2->fix("image.i.cena");
        ImageSolver solver1(*params2);
        solver1.addNormalEquations(ne);
        solver1.solveNormalEquations(q);
      }
    };

  }
}
