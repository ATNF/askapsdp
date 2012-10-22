/// @file AsciiTableAccessor.h
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

#ifndef ASKAP_CP_PIPELINETASKS_ASCIITABLEACCESSOR_H
#define ASKAP_CP_PIPELINETASKS_ASCIITABLEACCESSOR_H

// System includes
#include <string>
#include <istream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <list>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "Common/ParameterSet.h"
#include "casa/aipstype.h"
#include "casa/Quanta/Quantum.h"
#include "casa/Quanta/Unit.h"
#include "skymodelclient/Component.h"

// Local package includes
#include "cmodel/IGlobalSkyModel.h"

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief An object providing access to a sky model contained in a
/// row/column (space delimited) ASCII test file
class AsciiTableAccessor : public IGlobalSkyModel {
    public:
        /// Constructor
        /// @param[in] filename name of the file containing the source catalog
        AsciiTableAccessor(const std::string& filename,
                           const LOFAR::ParameterSet& parset);

        /// Constructor
        /// Used for testing only, so a stringstream can be
        /// passed in.
        /// @param[in] stream   istream from which data will be read.
        AsciiTableAccessor(const std::stringstream& sstream,
                           const LOFAR::ParameterSet& parset);

        /// Destructor
        virtual ~AsciiTableAccessor();

        /// @see askap::cp::pipelinetasks::IGlobalSkyModel::coneSearch
        virtual std::vector<askap::cp::skymodelservice::Component> coneSearch(
                const casa::Quantity& ra,
                const casa::Quantity& dec,
                const casa::Quantity& searchRadius,
                const casa::Quantity& fluxLimit);

    private:

        // Enumerates the required and optional fields
        enum FieldEnum {
            RA,
            DEC,
            FLUX,
            MAJOR_AXIS,
            MINOR_AXIS,
            POSITION_ANGLE,
            SPECTRAL_INDEX,     // Optional
            SPECTRAL_CURVATURE  // Optional
        };

        typedef std::map< FieldEnum, std::pair< short, casa::Unit > > FieldDesc;

        // Process a single (non comment) line of the input file.
        // This method processes a line, building a component instance
        // for each component which meets the search radius and flux limit
        // criteria. The component is then added to the list.
        void processLine(const std::string& line,
                const casa::Quantity& searchRA,
                const casa::Quantity& searchDec,
                const casa::Quantity& searchRadius,
                const casa::Quantity& fluxLimit,
                std::list<askap::cp::skymodelservice::Component>& list);

        static std::pair< short, casa::Unit > makeFieldDescEntry(
                const LOFAR::ParameterSet& parset,
                const std::string& colkey,
                const std::string& unitskey);

        void initFieldDesc(const LOFAR::ParameterSet& parset);

        // File stream from which components will be read
        boost::scoped_ptr<std::istream> itsFile;

        // Count of components below the flux limit
        casa::uLong itsBelowFluxLimit;

        // Count of components outside of the search radius
        casa::uLong itsOutsideSearchCone;

        // Field description
        FieldDesc itsFields;
};

}
}
}

#endif
