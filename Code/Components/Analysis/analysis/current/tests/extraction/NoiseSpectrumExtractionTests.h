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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <extraction/NoiseSpectrumExtractor.h>
#include <sourcefitting/RadioSource.h>
#include <cppunit/extensions/HelperMacros.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>
#include <Common/KVpair.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <imageaccess/CasaImageAccess.h>
#include <duchamp/Detection/finders.hh>
#include <duchamp/PixelMap/Object2D.hh>
#include <string>
#include <math.h>

//ASKAP_LOGGER(logger, ".noiseSpectrumExtractionTest");
namespace askap {

  namespace analysis {


    class NoiseSpectrumExtractionTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE(NoiseSpectrumExtractionTest);
      CPPUNIT_TEST(readParset);
      CPPUNIT_TEST_SUITE_END();

    private:
      NoiseSpectrumExtractor extractor;
      LOFAR::ParameterSet parset; // used for defining the subdef
      std::string tempImage;
      std::string outfile;
      RadioSource object;

    public:

      void setUp() {

      }

      void readParset(){

      }

      void tearDown() {

      }

    };

  }
}
