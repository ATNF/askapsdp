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
#include "distributedimager/IPreDifferTask.h"
#include "distributedimager/IBasicComms.h"

namespace askap {
    namespace cp {

        class PreDifferMaster : public IPreDifferTask
        {
            public:
                PreDifferMaster(LOFAR::ACC::APS::ParameterSet& parset,
                        askap::cp::IBasicComms& comms);
                virtual ~PreDifferMaster();

                virtual askap::scimath::INormalEquations::ShPtr calcNE(askap::scimath::Params::ShPtr model_p);

            private:

                /// @brief Utility function to get dataset names from parset.
                ///     
                /// Given a ParameterSet, return a vector containing all the datasets
                /// specified. This function will look for datasets in the Cimager manner:
                /// @code
                /// Cimager.dataset        = [10uJy_stdtest_0.ms,10uJy_stdtest_1.ms]
                /// @endcode
                ///     
                /// It also supports another method which is necessary for the specification
                /// of large numbers of datasets:
                /// @code           
                /// Cimager.dataset0                             = 10uJy_stdtest_0.ms
                /// Cimager.dataset1                             = 10uJy_stdtest_1.ms
                /// <and so on>
                /// @endcode
                ///     
                /// @param[in] parset   the parameterset to use as input.
                /// @return a vector containing in each element one dataset.
                std::vector<std::string> getDatasets(LOFAR::ACC::APS::ParameterSet& itsParset);

                /// Normal equations
                askap::scimath::INormalEquations::ShPtr itsNormalEquation_p;

                /// Parameter set
                LOFAR::ACC::APS::ParameterSet& itsParset;

                /// Communications class
                askap::cp::IBasicComms& itsComms;

                /// Model
                askap::scimath::Params::ShPtr itsModel;

                // No support for assignment
                PreDifferMaster& operator=(const PreDifferMaster& rhs);

                // No support for copy constructor
                PreDifferMaster(const PreDifferMaster& src);
        };

    };
};

#endif
