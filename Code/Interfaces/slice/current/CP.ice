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
#include <IService.ice>

module askap
{
module interfaces
{
module cp
{
    /**
     * Used to indicate the specified scheduling block does not exist
     */
    exception NoSuchSchedulingBlockException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Used to indicate the pipeline(s) is/are already running.
     */
    exception AlreadyRunningException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Used to indicate the a pipeline failed to start due to some
     * problem other than the above.
     */
    exception PipelineStartException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Observing Service
     *
     * This service is used in association with the Telescope Operating System
     * to carry out an observation.
     */
    interface ICPObsService extends askap::interfaces::services::IService
    {
        /**
         * Calling this method instructs the central processor to carry out the
         * observation described in the parameter set associated with the
         * scheduling block. This includes both data ingest and data processing
         * (i.e. running the ingest and processing pipelines).
         * 
         * This method, when called will block until such times as the central
         * processor is ready to begin receiving data from the correlator and
         * telescope operating system. The central processor takes some time to
         * prepare for an observation (on the order of a few seconds), hence the
         * need to indicate when it is ready by blocking.
         *
         * @throws NoSuchSchedulingBlockException   if the schheduling block id
         *              is not valid (i.e. not known to the data service)
         * @throws AlreadyRunningException  if an observation is already in
         *              in progress. This observation must either be aborted or
         *              left to conclude normally.
         * @throws PipelineStartException one or more pipelines failed to start.
         */
        void startObs(long sbid) throws NoSuchSchedulingBlockException,
            AlreadyRunningException, PipelineStartException;

        /**
         * Calling this method instructs the central processor to abort the
         * current observation. This stops the data acquisition process,
         * however does not necessarily prevent the observation from moving to
         * the post-processing stage. The configuration parameters in the 
         * scheduling block indicate if data processing should be attempted
         * for an aborted observation.
         *
         * This method is non-blocking. To wait until the observation has been
         * aborted, follow this call with a call to waitObs().
         */
        void abortObs();

        /**
         * Blocks until the observation in progress is completed. Specifically,
         * until the ingest pipeline finishes, either successfully or with
         * error. When this method returns with a return value of true, the
         * central processor is ready to start a new observation.
         *
         * If called while no observation is in progress, it simply returns
         * true immediately.
         *
         * @param timeout the maximum time to wait in milliseconds. Once the
         *                timeout expires, if the observation is still running
         *                this call will return "false". A value of -1 will
         *                block until the observation is complete, and a value
         *                of 0 will provide non-blocking semantics, allowing
         *                the caller to simply determine if an observation is
         *                running or not.
         * @return true if the observation is complete (or not running when called)
         *         and false if the timeout limit is reached and the observation
         *         is still running.
         */
        bool waitObs(long timeout);
    };

};
};
};

#endif
