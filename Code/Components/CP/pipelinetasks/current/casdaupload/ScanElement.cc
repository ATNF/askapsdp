/// @file ScanElement.cc
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

// Include own header file first
#include "casdaupload/ScanElement.h"

// Include package level header file
#include "askap_pipelinetasks.h"

// System includes
#include <string>
#include <sstream>
#include <cmath>
#include <stdint.h>

// ASKAPsoft includes
#include "askap/AskapUtil.h"
#include "xercesc/dom/DOM.hpp" // Includes all DOM
#include "votable/XercescUtils.h"
#include "votable/XercescString.h"

// Using
using namespace std;
using namespace askap::cp::pipelinetasks;
using namespace casa;
using namespace xercesc;
using askap::utility::toString;
using askap::accessors::XercescUtils;
using askap::accessors::XercescString;

ScanElement::ScanElement(const int id,
                         const casa::MEpoch& scanstart,
                         const casa::MEpoch& scanend,
                         const casa::MDirection& fieldcentre,
                         const std::string& fieldname,
                         const casa::Vector<casa::Int> polarisations,
                         const int numchan,
                         const casa::Quantity& centrefreq,
                         const casa::Quantity& channelwidth)
    : itsId(id),
      itsScanStart(scanstart),
      itsScanEnd(scanend),
      itsFieldCentre(fieldcentre),
      itsFieldName(fieldname),
      itsPolarisations(polarisations),
      itsNumChan(numchan),
      itsCentreFreq(centrefreq),
      itsChannelWidth(channelwidth)
{
}

xercesc::DOMElement* ScanElement::toXmlElement(xercesc::DOMDocument& doc) const
{
    DOMElement* e = doc.createElement(XercescString("scan"));

    XercescUtils::addTextElement(*e, "id", toString(itsId));
    XercescUtils::addTextElement(*e, "scanstart",
                                 MVTime(itsScanStart.get("s")).string(MVTime::FITS));
    XercescUtils::addTextElement(*e, "scanend",
                                 MVTime(itsScanEnd.get("s")).string(MVTime::FITS));

    // Format the field direction like so: "1.13701, -1.112"
    stringstream ss;
    ss << itsFieldCentre.getAngle().getValue("rad")(0);
    ss << ", ";
    ss << itsFieldCentre.getAngle().getValue("rad")(1);

    DOMElement* child = XercescUtils::addTextElement(*e, "fieldcentre", ss.str());
    child->setAttribute(XercescString("units"), XercescString("rad"));
    XercescUtils::addTextElement(*e, "coordsystem", itsFieldCentre.getRefString());
    XercescUtils::addTextElement(*e, "fieldname", itsFieldName);

    // Format the polarisaztions like so: "XX, XY, YX, YY"
    ss.str("");
    for (size_t i = 0; i < itsPolarisations.size(); ++i) {
        if (i != 0) ss << ", ";
        ss << Stokes::name(Stokes::type(itsPolarisations(i)));
    }

    XercescUtils::addTextElement(*e, "polarisations", ss.str());

    XercescUtils::addTextElement(*e, "numchan", toString(itsNumChan));

    const string FREQ_UNITS = "Hz";
    child = XercescUtils::addTextElement(*e, "centrefreq",
                                         toString(static_cast<uint64_t>(round(fabs(itsCentreFreq.getValue(FREQ_UNITS.c_str()))))));
    child->setAttribute(XercescString("units"), XercescString(FREQ_UNITS));

    child = XercescUtils::addTextElement(*e, "chanwidth",
                                         toString(static_cast<uint64_t>(round(fabs(itsChannelWidth.getValue(FREQ_UNITS.c_str()))))));
    child->setAttribute(XercescString("units"), XercescString(FREQ_UNITS));

    return e;
}
