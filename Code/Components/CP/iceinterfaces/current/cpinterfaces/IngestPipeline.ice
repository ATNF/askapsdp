// @file IngestPipeline.ice
//
// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_CP_INGESTPIPELINE_ICE
#define ASKAP_CP_INGESTPIPELINE_ICE

//#include <CommonTypes.ice>

module askap {
    module cpinterfaces {

        /**
         * This exception is thrown by the methods of the IngestPipeline
         * interface.
         **/
        //exception IngestException extends askap::interfaces::AskapIceException
        //{
        //};

        /**
         * This is the interface fulfilled by the IngestPipeline (internal
         * to the central processor).
         */
        //interface IIngestPipeline {

            /**
             * Calling this method instructs the ingest pipeline instance to
             * configure itself per the attached parameter map. Typically a
             * call to start() is made when preparing to carry out an
             * observation.
             *
             * This method, when called will block until such times as the ingest
             * pipeline is ready to begin receiving data from the correlator and
             * telescope operating system. The ingest pipeline takes some time to
             * prepare for an observation (on the order of a few seconds), hence
             * the need to indicate when it is ready by blocking.
             *
             * @param parmap configuration parameters which the ingest pipeline
             *               should use.
             *
             * @throws IngestException if the startup cannot be completed,
             *          typically due to a problem with the configuration
             *          (parmap) that was passed, a problem with the ingest
             *          process itself, or if the ingest pipeline is already
             *          running. The "reason" member of the exception object
             *          will indicate the reason for the exception.
             */
            //["ami"] void start(askap::interfaces::ParameterMap parmap)
            //    throws IngestException;

            /**
             * Wait until the current ingest activity has completed. This
             * method is blocking.
             *
             * Returns in the case of an observation completing without
             * error. Otherwise throws an exception.
             *
             * @throws IngestException indicates an error occured which
             * resulted in the premature termination of the ingest activity.
             * The "reason" member of the exception object will indicate
             * the nature of the error which led to premature termination.
             */
            //["ami"] void wait()
            //    throws IngestException;

            /**
             * Calling this method instructs the ingest pipeline instance to
             * abort the current ingest activity.
             *
             * This method will block until the ingest has been aborted and
             * the ingest pipeline instance is ready to begin a new observation.
             *
             * @throws IngestException if the abort process could not be
             *          completed because no ingest task is currently
             *          executing.
             */
            //["ami"] void abort()
            //    throws IngestException;

        //};

    };
};

#endif
