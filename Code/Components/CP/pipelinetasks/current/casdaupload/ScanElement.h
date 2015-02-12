/// @file ScanElement.h
///
/// @copyright (c) 2015 CSIRO
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

#ifndef ASKAP_CP_PIPELINETASKS_SCAN_ELEMENT_H
#define ASKAP_CP_PIPELINETASKS_SCAN_ELEMENT_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "casa/Quanta/MVTime.h"
#include "casa/Quanta/Quantum.h"
#include "measures/Measures/MDirection.h"
#include "measures/Measures/Stokes.h"
#include "measures/Measures/MEpoch.h"
#include "casa/Arrays/Vector.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM

// Local package includes

namespace askap {
namespace cp {
namespace pipelinetasks {

class ScanElement {
    public:
        ScanElement(const int id,
                    const casa::MEpoch& scanstart,
                    const casa::MEpoch& scanend,
                    const casa::MDirection& fieldcentre,
                    const std::string& fieldname,
                    const casa::Vector<casa::Int> polarisations,
                    const int numchan,
                    const casa::Quantity& centrefreq,
                    const casa::Quantity& channelwidth);

        xercesc::DOMElement* toXmlElement(xercesc::DOMDocument& doc) const;

    private:
        int itsId;
        casa::MEpoch itsScanStart;
        casa::MEpoch itsScanEnd;
        casa::MDirection itsFieldCentre;
        std::string itsFieldName;
        casa::Vector<casa::Int> itsPolarisations;
        int itsNumChan;
        casa::Quantity itsCentreFreq;
        casa::Quantity itsChannelWidth;
};

}
}
}

#endif
