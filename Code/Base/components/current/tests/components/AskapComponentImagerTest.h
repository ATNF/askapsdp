/// @file AskapComponentImagerTest.cc
///
/// @copyright (c) 2011 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// System includes
#include <limits>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include "askap/AskapLogging.h"
#include "askap/AskapError.h"
#include "askap/Log4cxxLogSink.h"
#include "casa/aipstype.h"
#include "casa/Arrays/IPosition.h"
#include "casa/Quanta.h"
#include "casa/Quanta/Quantum.h"
#include "images/Images/TempImage.h"
#include "images/Images/PagedImage.h"
#include "measures/Measures/MDirection.h"
#include "lattices/Lattices/TiledShape.h"
#include "components/ComponentModels/SkyComponent.h"
#include "components/ComponentModels/ComponentList.h"
#include "components/ComponentModels/Flux.h"
#include "components/ComponentModels/ConstantSpectrum.h"
#include "components/ComponentModels/SpectralIndex.h"
#include "components/ComponentModels/PointShape.h"
#include "components/ComponentModels/GaussianShape.h"
#include "coordinates/Coordinates/CoordinateSystem.h"
#include "coordinates/Coordinates/DirectionCoordinate.h"
#include "coordinates/Coordinates/SpectralCoordinate.h"
#include "coordinates/Coordinates/StokesCoordinate.h"

// Classes to test
#include "components/AskapComponentImager.h"

// Using
using namespace askap;
using namespace askap::components;
using namespace casa;
using namespace std;

namespace askap {
namespace components {

class AskapComponentImagerTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(AskapComponentImagerTest);
        CPPUNIT_TEST(testFourPols);
        CPPUNIT_TEST(testGaussian);
        CPPUNIT_TEST(testTaylorTerms);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testFourPols() {
            ComponentList list;

            // Centre of the image
            const MDirection dir(casa::Quantity(187.5, "deg"),
                    casa::Quantity(-45.0, "deg"),
                    MDirection::J2000);

            // Create a component at the image centre with constant spectrum
            Vector<Double> fluxVals(4);
            fluxVals(0) = 1.0; 
            fluxVals(1) = 0.7; 
            fluxVals(2) = 0.6; 
            fluxVals(3) = 0.5; 
            const Flux<casa::Double> flux(fluxVals(0), fluxVals(1), fluxVals(2), fluxVals(3));
            {
                const ConstantSpectrum spectrum;
                const PointShape shape(dir);
                list.add(SkyComponent(flux, shape, spectrum));
            }

            // Create another, offset from the centre, with a spectral index
            // spectral model
            SpectralIndex spectrum2(MFrequency(Quantity(850, "MHz")), -0.7);
            {
                const PointShape shape(MDirection(
                            casa::Quantity(187.5, "deg"),
                            casa::Quantity(-45.02, "deg"),
                            MDirection::J2000));
                list.add(SkyComponent(flux, shape, spectrum2));
            }

            Vector<Int> iquv(4);
            iquv(0) = Stokes::I; iquv(1) = Stokes::Q;
            iquv(2) = Stokes::U; iquv(3) = Stokes::V;
            TempImage<Float> image = createImage<Float>(dir, 256, 256, iquv);
            AskapComponentImager::project(image, list);

            // Check the one pixel has the expected flux
            const double tolerance = 1e-7;
            const Double scale = spectrum2.sample(MFrequency(Quantity(1400, "MHz")));
            for (uInt pol = 0; pol < iquv.size(); ++pol) {
                const IPosition pixelPos1(4, 128, 128, 0, pol);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(fluxVals(pol), image.getAt(pixelPos1), tolerance);

                const IPosition pixelPos2(4, 128, 114, 0, pol);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(fluxVals(pol) * scale,
                            image.getAt(pixelPos2), tolerance);
            }

            // Uncomment the below two lines to write out the image
            //PagedImage<Float> pimage(image.shape(), image.coordinates(), "image.unittest_pointshape.casa");
            //pimage.copyData(image);
        }

        void testGaussian() {
            ComponentList list;

            // Centre of the image
            const MDirection dir(casa::Quantity(187.5, "deg"),
                    casa::Quantity(-45.0, "deg"),
                    MDirection::J2000);

            // Create a component at the image centre with constant spectrum
            const Flux<casa::Double> flux(1.0);
            const ConstantSpectrum spectrum;
            const GaussianShape shape(dir,
                    casa::Quantity(12.0, "arcsec"),
                    casa::Quantity(6.0, "arcsec"),
                    casa::Quantity(0, "deg"));
            list.add(SkyComponent(flux, shape, spectrum));

            Vector<Int> iquv(1);
            iquv(0) = Stokes::I;
            TempImage<Float> image = createImage<Float>(dir, 256, 256, iquv);
            AskapComponentImager::project(image, list);

            // Test there is some flux in the centre pixel
            const IPosition pixelPos1(4, 128, 128, 0, 0);
            CPPUNIT_ASSERT(image.getAt(pixelPos1) > 0.0);

            // Uncomment the below two lines to write out the image
            //PagedImage<Float> pimage(image.shape(), image.coordinates(), "image.unittest_gaussian.casa");
            //pimage.copyData(image);
        }

        void testTaylorTerms() {
            ComponentList list;

            // Centre of the image
            const MDirection dir(casa::Quantity(187.5, "deg"),
                    casa::Quantity(-45.0, "deg"),
                    MDirection::J2000);

            // Create a component at the image centre with spectral index
            const Flux<casa::Double> flux(1.0);
            const Double spectralIndex = -0.7;
            {
                const SpectralIndex spectrum(MFrequency(Quantity(850, "MHz")), spectralIndex);
                const PointShape shape(dir);
                list.add(SkyComponent(flux, shape, spectrum));
            }

            Vector<Int> iquv(1);
            iquv(0) = Stokes::I;

            // Test all terms
            TempImage<Float> image = createImage<Float>(dir, 256, 256, iquv);
            for (unsigned int i = 0; i < 3; ++i) {
                image.set(0.0);
                AskapComponentImager::project(image, list, i);
            }

            // Check terms > 2 result in an exception being thrown
            image.set(0.0);
            CPPUNIT_ASSERT_THROW(AskapComponentImager::project(image, list, 3),
                    askap::AskapError);
        }

    private:
        casa::CoordinateSystem createCoordinateSystem(const casa::uInt nx, const casa::uInt ny,
            const Vector<Int>& stokes)
        {
            CoordinateSystem coordsys;

            // Direction Coordinate
            {
                Matrix<Double> xform(2, 2);
                xform = 0.0;
                xform.diagonal() = 1.0;
                const Quantum<Double> ra(187.5, "deg");
                const Quantum<Double> dec(-45.0, "deg");

                const Quantum<Double> xcellsize(5.0 * -1.0, "arcsec");
                const Quantum<Double> ycellsize(5.0, "arcsec");

                const DirectionCoordinate radec(MDirection::J2000, Projection(Projection::SIN),
                        ra, dec, xcellsize, ycellsize, xform, nx / 2, ny / 2);

                coordsys.addCoordinate(radec);
            }

            // Spectral Coordinate
            {
                const Quantum<Double> f0(1400.0, "MHz");
                const Quantum<Double> inc(300.0, "MHz");
                const Double refPix = 0.0;  // is the reference pixel
                const SpectralCoordinate sc(MFrequency::TOPO, f0, inc, refPix);

                coordsys.addCoordinate(sc);
            }

            // Stokes Coordinate
            {
                const StokesCoordinate stokescoord(stokes);
                coordsys.addCoordinate(stokescoord);
            }

            return coordsys;
        }

        template <class T>
        casa::TempImage<T> createImage(const MDirection& dir,
            const uInt nx, const uInt ny, const Vector<Int>& stokes) {

            // Create the image
            IPosition imgShape(4, nx, ny, 1, stokes.size());
            CoordinateSystem coordsys = createCoordinateSystem(nx, ny, stokes);
            casa::TempImage<T> image(TiledShape(imgShape), coordsys);
            image.set(0.0);

            // Set brightness units
            image.setUnits(casa::Unit("Jy/pixel"));
            return image;
        }
};

}   // End namespace components
}   // End namespace askap
