/// @file
///
/// DistributedImageMultiScaleSolverWorker
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
///
#ifndef CP_DISTRIBUTEDIMAGEMULTISCALESOLVERWORKER_H_
#define CP_DISTRIBUTEDIMAGEMULTISCALESOLVERWORKER_H_

// ASKAPsoft includes
#include <APS/ParameterSet.h>

// Local includes
#include <distributedimager/IBasicComms.h>

namespace askap
{
    namespace cp
    {
        class DistributedImageMultiScaleSolverWorker 
        {
            public:
                DistributedImageMultiScaleSolverWorker(const LOFAR::ACC::APS::ParameterSet& parset,
                        askap::cp::IBasicComms& comms);

                void solveNormalEquations(void);

            private:

                // ID of the master process
                static const int cg_master = 0;

                // Parameter set
                LOFAR::ACC::APS::ParameterSet m_parset;

                // Communications class
                askap::cp::IBasicComms& m_comms;

        };

    }
}
#endif
