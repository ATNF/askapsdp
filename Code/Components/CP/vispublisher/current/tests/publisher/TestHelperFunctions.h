/// @file TestHelperFunctions.cc
///
/// @copyright (c) 2014 CSIRO
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

// Support classes
#include <stdint.h>
#include <limits>
#include <complex>
#include <algorithm>
#include <vector>
#include <cstring>
#include "casa/Arrays/Cube.h"
#include "askap/AskapError.h"
#include "publisher/InputMessage.h"

namespace askap {
namespace cp {
namespace vispublisher {

class TestHelperFunctions {

    public:
        static InputMessage createInputMessage()
        {
            InputMessage msg;

            msg.timestamp() = 1234;
            msg.nRow() = N_BEAM * N_BASELINE;
            msg.nPol() = N_POL;
            msg.nChannels() = N_CHAN;
            msg.chanWidth() = 18.518 * 1000;

            msg.frequency().push_back(1.0);
            msg.frequency().push_back(2.0);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(N_CHAN),
                    msg.frequency().size());

            // Row 1
            msg.antenna1().push_back(0);
            msg.antenna2().push_back(1);
            msg.beam().push_back(0);

            // Row 2
            msg.antenna1().push_back(0);
            msg.antenna2().push_back(2);
            msg.beam().push_back(0);

            // Row 3
            msg.antenna1().push_back(1);
            msg.antenna2().push_back(2);
            msg.beam().push_back(0);

            // Row 4
            msg.antenna1().push_back(0);
            msg.antenna2().push_back(1);
            msg.beam().push_back(1);

            // Row 5
            msg.antenna1().push_back(0);
            msg.antenna2().push_back(2);
            msg.beam().push_back(1);

            // Row 6
            msg.antenna1().push_back(1);
            msg.antenna2().push_back(2);
            msg.beam().push_back(1);

            // Stokes
            for (uint32_t pol = 0; pol < N_POL; ++pol) {
                msg.stokes().push_back(pol);
            }

            // Visibilities and Flag
            const size_t N_ROW = N_BASELINE * N_BEAM;
            casa::Cube< std::complex<float> > vis(N_ROW, N_CHAN, N_POL);
            casa::Cube<uint8_t> flag(N_ROW, N_CHAN, N_POL);

            for (size_t row = 0; row < N_ROW; ++row) {
                for (size_t chan = 0; chan < N_CHAN; ++chan) {
                    for (size_t pol = 0; pol < N_POL; ++pol) {
                        flag(row, chan, pol) = 1;
                        vis(row, chan, pol) = visgen(chan, msg.antenna1()[row],
                                msg.antenna2()[row], msg.beam()[row], pol);
                    }
                }
            }
            std::vector< std::complex<float> >& msgvis = msg.visibilities();
            msgvis.resize(vis.size());
            std::copy(vis.begin(), vis.end(), msgvis.begin());
            std::vector<uint8_t>& msgflag = msg.flag();
            msgflag.resize(flag.size());
            std::copy(flag.begin(), flag.end(), msgflag.begin());

            return msg;
        }

        /// Generates visibilities based on indexing information. Is used to populate
        /// the data structure and then as expected values
        static std::complex<float> visgen(uint32_t chan, uint32_t ant1,
                uint32_t ant2, uint32_t beam, uint32_t pol) {
            // Note: the "magic numbers" below are just some prime numbers
            float val = chan * 433;
            val += ant1 * 809;
            val += ant2 * 929;
            val += beam * 67;
            val += pol * 347;
            return val;
        }

        static const uint32_t N_BEAM;
        static const uint32_t N_CHAN;
        static const uint32_t N_POL;
        static const uint32_t N_BASELINE;
};

const uint32_t TestHelperFunctions::N_BEAM = 2;
const uint32_t TestHelperFunctions::N_CHAN = 2;
const uint32_t TestHelperFunctions::N_POL = 4;
const uint32_t TestHelperFunctions::N_BASELINE = 3;

}   // End namespace vispublisher
}   // End namespace cp
}   // End namespace askap
