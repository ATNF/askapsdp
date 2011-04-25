/// @file IceMapper.h
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

#ifndef ASKAP_CP_CALDATASERVICE_ICEMAPPER_H
#define ASKAP_CP_CALDATASERVICE_ICEMAPPER_H

// ASKAPsoft includes
#include "CalibrationDataService.h" // Ice generated interface

// Local package includes
#include "calibrationclient/JonesIndex.h"
#include "calibrationclient/JonesJTerm.h"
#include "calibrationclient/GenericSolution.h"

namespace askap {
namespace cp {
namespace caldataservice {

class IceMapper {

    public:

        static askap::interfaces::calparams::TimeTaggedGainSolution toIce(const askap::cp::caldataservice::GainSolution& sol);
        static askap::interfaces::calparams::TimeTaggedLeakageSolution toIce(const askap::cp::caldataservice::LeakageSolution& sol);
        static askap::interfaces::calparams::TimeTaggedBandpassSolution toIce(const askap::cp::caldataservice::BandpassSolution& sol);

        static askap::cp::caldataservice::GainSolution fromIce(const askap::interfaces::calparams::TimeTaggedGainSolution& ice_sol);
        static askap::cp::caldataservice::LeakageSolution fromIce(const askap::interfaces::calparams::TimeTaggedLeakageSolution& ice_sol);
        static askap::cp::caldataservice::BandpassSolution fromIce(const askap::interfaces::calparams::TimeTaggedBandpassSolution& ice_sol);

    private:
        static askap::interfaces::DoubleComplex toIce(const casa::DComplex& val);
        static askap::interfaces::calparams::JonesIndex toIce(const askap::cp::caldataservice::JonesIndex& jindex);
        static askap::interfaces::calparams::JonesJTerm toIce(const askap::cp::caldataservice::JonesJTerm& jterm);

        static casa::DComplex fromIce(const askap::interfaces::DoubleComplex& ice_val);
        static askap::cp::caldataservice::JonesJTerm fromIce(const askap::interfaces::calparams::JonesJTerm& ice_jterm);
        static askap::cp::caldataservice::JonesIndex fromIce(const askap::interfaces::calparams::JonesIndex& ice_jindex);
};

};
};
};

#endif
