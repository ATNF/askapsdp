/// @file ParsetConfiguration.h
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_PARSETCONFIGURATION_H
#define ASKAP_CP_PARSETCONFIGURATION_H

// System includes
#include <string>

// ASKAPsoft includes
#include "Common/ParameterSet.h"
#include "casa/aips.h"
#include "casa/Quanta.h"
#include "casa/Arrays/Matrix.h"
#include "casa/Arrays/Vector.h"
#include "measures/Measures/MPosition.h"

// Local package includes
#include "ingestutils/IConfiguration.h"

namespace askap {
namespace cp {

class ParsetConfiguration : public IConfiguration {
    public:
        ParsetConfiguration(const LOFAR::ParameterSet& parset);

        virtual void getAntennas(casa::Vector<std::string>& names,
                                 std::string& station,
                                 casa::Matrix<double>& antXYZ,
                                 casa::Matrix<double>& offset,
                                 casa::Vector<casa::Double>& dishDiameter,
                                 casa::Vector<std::string>& mount);

        virtual void getFeeds(casa::String& mode,
                              casa::Vector<double>& x,
                              casa::Vector<double>& y,
                              casa::Vector<casa::String>& pol);

        virtual void getSpWindows(casa::String& spWindowName, int& nChan,
                                  casa::Quantity& startFreq, casa::Quantity& freqInc,
                                  casa::String& stokesString);

    private:

        casa::Quantity asQuantity(const std::string& str);

        // Parameter set
        const LOFAR::ParameterSet itsParset;
};

}
}

#endif
