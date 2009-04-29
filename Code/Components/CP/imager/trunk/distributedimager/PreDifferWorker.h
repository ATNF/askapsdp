/// @file PreDifferWorker.h
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

#ifndef ASKAP_CP_PREDIFFERWORKER_H
#define ASKAP_CP_PREDIFFERWORKER_H

// System includes

// ASKAPsoft includes
#include <APS/ParameterSet.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>
#include <gridding/IVisGridder.h>

// Local includes
#include "distributedimager/IPreDifferTask.h"
#include "distributedimager/IBasicComms.h"

namespace askap {
    namespace cp {

        class PreDifferWorker : public IPreDifferTask
        {
            public:
                PreDifferWorker(LOFAR::ACC::APS::ParameterSet& parset,
                        askap::cp::IBasicComms& comms);
                virtual ~PreDifferWorker();

                virtual askap::scimath::INormalEquations::ShPtr calcNE(askap::scimath::Params::ShPtr notused);

            private:
                // Reduce these normal equations down to the master (id 0). Ideally
                // this reduction would be a graph style reduction and not just send
                // all normal equations to the master.
                void reduceNE(askap::scimath::INormalEquations::ShPtr ne_p, int count);

                // Parameter Set
                LOFAR::ACC::APS::ParameterSet& m_parset;

                // Communications class
                askap::cp::IBasicComms& m_comms;

                // Pointer to the gridder
                askap::synthesis::IVisGridder::ShPtr m_gridder_p;

                // Normal equations
                askap::scimath::INormalEquations::ShPtr m_ne_p;

                // ID of the master process
                static const int cg_master = 0;
        };

    };
};

#endif
