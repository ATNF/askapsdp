/// @file SpectralLineMaster.h
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

#ifndef ASKAP_CP_SIMAGER_SPECTRALLINEMASTER_H
#define ASKAP_CP_SIMAGER_SPECTRALLINEMASTER_H

// System includes
#include <string>
#include <vector>

// ASKAPsoft includes
#include <boost/scoped_ptr.hpp>
#include <Common/ParameterSet.h>
#include <fitting/Params.h>

// Local includes
#include "distributedimager/IBasicComms.h"
#include "distributedimager/CubeBuilder.h"

namespace askap {
    namespace cp {

        class SpectralLineMaster
        {
            public:
                SpectralLineMaster(LOFAR::ParameterSet& parset,
                        askap::cp::IBasicComms& comms);
                ~SpectralLineMaster();

                void run(void);


            private:

                struct MSInfo
                {
                    casa::uInt nChan;
                    std::vector<casa::Quantity> freqs;
                };

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
                std::vector<std::string> getDatasets(const LOFAR::ParameterSet& itsParset);

                void handleImageParams(askap::scimath::Params::ShPtr params, unsigned int chan);

                static MSInfo getMSInfo(const std::string& ms);

                static std::vector<MSInfo> getMSInfo(const std::vector<std::string>& ms);

                static casa::uInt getNumChannels(const std::vector<MSInfo>& info);

                static casa::Quantity getFirstFreq(const std::vector<MSInfo>& info);

                static casa::Quantity getFreqInc(const std::vector<MSInfo>& info);

                /// Parameter set
                LOFAR::ParameterSet& itsParset;

                /// Communications class
                askap::cp::IBasicComms& itsComms;

                boost::scoped_ptr<CubeBuilder> itsImageCube;
                boost::scoped_ptr<CubeBuilder> itsPSFCube;
                boost::scoped_ptr<CubeBuilder> itsResidualCube;
                boost::scoped_ptr<CubeBuilder> itsWeightsCube;

                // No support for assignment
                SpectralLineMaster& operator=(const SpectralLineMaster& rhs);

                // No support for copy constructor
                SpectralLineMaster(const SpectralLineMaster& src);
        };

    };
};

#endif
