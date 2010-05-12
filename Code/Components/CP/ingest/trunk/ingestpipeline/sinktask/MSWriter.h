/// @file MSWriter.h
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

#ifndef ASKAP_CP_WRITER_H
#define ASKAP_CP_WRITER_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include "boost/scoped_ptr.hpp"
#include "casa/aips.h"
#include "scimath/Mathematics/RigidVector.h"
#include "ms/MeasurementSets/MeasurementSet.h"
#include "casa/Arrays/Vector.h"
#include "casa/Arrays/Matrix.h"
#include "measures/Measures/Measure.h"
#include "measures/Measures/MEpoch.h"
#include "measures/Measures/MPosition.h"
#include "measures/Measures/MDirection.h"


namespace askap {
namespace cp {

class MSWriter {
    public:
        MSWriter(const std::string& filename,
                unsigned int bucketSize,
                unsigned int tileNcorr,
                unsigned int tileNchan);
        ~MSWriter();

        casa::uInt addAntennaRow(const std::string& name,
                const std::string& station,
                const std::string& type,
                const std::string& mount,
                const casa::MPosition& position,
                const casa::MPosition& offset,
                const casa::Double& dishDiameter);

        casa::uInt addDataDescRow(const casa::Int spwId,
                                  const casa::Int polId);

        casa::uInt addFeedRow(const casa::Int antennaId,
                const casa::Int feedId,
                const casa::Int spwId,
                const casa::MEpoch& time,
                const casa::Double& interval,
                const casa::Int numReceptors,
                const casa::Int beamId,
                const casa::Vector<casa::MDirection>& beamOffset,
                const casa::Vector<std::string>& polarizationType,
                const casa::Matrix<casa::Complex> polResponse,
                const casa::MPosition& position,
                const casa::Vector<casa::Double>& receptorAngle);

        casa::uInt addFieldRow(const std::string& name,
                const std::string& code,
                const casa::MEpoch& time,
                const casa::Int numPoly,
                const casa::Vector<casa::MDirection>& delayDir,
                const casa::Vector<casa::MDirection>& phaseDir,
                const casa::Vector<casa::MDirection>& referenceDir,
                const casa::Int sourceId);

        casa::uInt addObservationRow(const std::string& telescopeName,
                const casa::RigidVector<casa::MEpoch, 2>& timeRange,
                const std::string& observer);

        casa::uInt addPointingRow(const casa::Int antennaId,
                const casa::MEpoch& time,
                const casa::Double& interval,
                const std::string& name,
                const casa::Int numPoly,
                const casa::MEpoch& timeOrigin,
                const casa::Vector<casa::MDirection>& direction,
                const casa::Vector<casa::MDirection>& target,
                const casa::Bool tracking);

        casa::uInt addPolarisationRow(const casa::Int numCorr,
                const casa::Vector<casa::Int>& corrType,
                const casa::RigidVector<casa::Int, 2>& corrProduct );

        casa::uInt addSpWindowRow(const casa::Int numChan,
                const std::string& name,
                const casa::Double& refFrequency,
                const casa::Vector<casa::Double>& chanFreq,
                const casa::Vector<casa::Double>& chanWidth,
                const casa::Vector<casa::Double>& effectiveBW,
                const casa::Vector<casa::Double>& resolution,
                const casa::Double& totalBandwidth,
                const casa::Int netSideband,
                const casa::Int ifConvChain,
                const casa::Int freqGroup,
                const std::string& freqGroupName);

        casa::uInt addMainRow(const casa::MEpoch& time,
                const casa::Int antenna1,
                const casa::Int antenna2,
                const casa::Int feed1,
                const casa::Int feed2,
                const casa::Int dataDescId,
                const casa::Int processorId,
                const casa::Int fieldId,
                const casa::Double interval, 
                const casa::Double exposure,
                const casa::MEpoch timeCentroid,
                const casa::Int scanNumber,
                const casa::Int arrayId,
                const casa::Int observationId,
                const casa::Int stateId,
                const casa::RigidVector<casa::Double, 3>& uvw,
                const casa::Matrix<casa::Complex>& data,
                const casa::Vector<casa::Float>& sigma,
                const casa::Vector<casa::Float>& weight,
                const casa::Matrix<casa::Bool>& flag);

    private:
        boost::scoped_ptr<casa::MeasurementSet> itsMs;

};

}
}

#endif
