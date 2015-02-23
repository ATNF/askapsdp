/// @file
///
/// Implementation of base class CatalogueEntry
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#include <catalogues/CatalogueEntry.h>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <Common/ParameterSet.h>

ASKAP_LOGGER(logger, ".catalogueentry");

namespace askap {

namespace analysis {

CatalogueEntry::CatalogueEntry(const LOFAR::ParameterSet &parset):
    itsSBid(parset.getString("SBid", "null"))
{
    std::string imageName = parset.getString("image");
    imageName.erase(imageName.rfind("."), std::string::npos);
    imageName.erase(0, imageName.rfind("/") + 1);
    std::stringstream id;
    id << "SB" << itsSBid << "_" << imageName << "_";
    itsIDbase = id.str();

}


}

}
