/// @file PreDifferMaster.h
///
/// @copyright (c) 2009 CSIRO
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

#ifndef ASKAP_CP_PREDIFFERMASTER_H
#define ASKAP_CP_PREDIFFERMASTER_H

// System includes

// ASKAPsoft includes
#include <APS/ParameterSet.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>

// Local includes
#include "distributedimager/IPreDiffer.h"
#include "distributedimager/IImagerComms.h"

namespace askap {
    namespace cp {

        class PreDifferMaster : public IPreDiffer
        {
            public:
                PreDifferMaster(LOFAR::ACC::APS::ParameterSet& parset,
                        askap::cp::IImagerComms& comms);
                virtual ~PreDifferMaster();

                virtual askap::scimath::INormalEquations::ShPtr calcNE(askap::scimath::Params::ShPtr model_p);

            private:
                // Normal equations
                askap::scimath::INormalEquations::ShPtr m_ne_p;

                // Parameter set
                LOFAR::ACC::APS::ParameterSet& m_parset;

                // Communications class
                askap::cp::IImagerComms& m_comms;

                // Model
                askap::scimath::Params::ShPtr m_model_p;
        };

    };
};

#endif
