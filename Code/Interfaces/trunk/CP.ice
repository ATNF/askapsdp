// @file CP.ice
//
// @copyright (c) 2009 CSIRO
// Australia Telescope National Facility (ATNF)
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// PO Box 76, Epping NSW 1710, Australia
// atnf-enquiries@csiro.au
//
// This file is part of the ASKAP software distribution.
//
// The ASKAP software distribution is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

#ifndef ASKAP_CP_ICE
#define ASKAP_CP_ICE

#include <CommonTypes.ice>

module askap
{
module interfaces
{
module cp
{
    /**
     * Observing Service
     *
     * This service is used in association with the Telescope Operating System
     * to carry out an observation.
     */
    interface ICPObsService
    {
        /**
         * Calling this method instructs the central processor to carry out the
         * observation described in the parameter map. This includes both data
         * ingest and data processing (i.e. running the ingest and processing
         * pipelines).
         * 
         * This method, when called will block until such times as the central
         * processor is ready to begin receiving data from the correlator and
         * telescope operating system. The central processor takes some time to
         * prepare for an observation (on the order of a few seconds), hence the
         * need to indicate when it is ready by blocking.
         */
        ["ami"] void startObs(askap::interfaces::ParameterMap parmap);

        /**
         * Calling this method instructs the central processor to abort the
         * current observation. This stops the data acquisition  process,
         * however does not necessarily prevent the observation from moving to
         * the post-processing stage. The configuration parameters passed to
         * the startObs() method indicate if data processing should be attempted
         * for an aborted observation.
         *
         * This method will block until the observation has been aborted and
         * the central processor is ready to start a new observation.
         */
        ["ami"] void abortObs();
    };

};
};
};

#endif
