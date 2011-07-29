// @file PksDataCapture.ice
//
// @copyright (c) 2011 CSIRO
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

#ifndef ASKAP_PKSDATACAPTURE_ICE
#define ASKAP_PKSDATACAPTURE_ICE

#include <CommonTypes.ice>

module askap
{
module interfaces
{
module pksdatacapture
{
    /**
     * Used to indicate data capture is already running.
     */
    exception AlreadyRunningException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Used to indicate the scheduling block ID is not valid
     */
    exception InvalidSBException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Parkes Data Capture Service
     */
    interface IPksDataCaptureService
    {
        /**
         * Calling this method instructs the data capture service to begin
         * capturing both telescope metadata and the ACMs.
         * 
         * This method, when called will block until such times as the service
         * is ready to begin receiving data from the beamformer and the
         * telescope observation manager.
         *
         * @throws AlreadyRunningException  if data capture is already in
         *              in progress. The stop() method must be called before
         *              calling start again().
         * @throws InvalidSBException  if the scheduling block id is less
         *                             than zero. 
         */
        ["ami"] void start(long sbid) throws AlreadyRunningException,
            InvalidSBException;

        /**
         * Calling this method instructs the data capture service to stop
         * the current data capture.
         *
         * This method will block until the data capture has been stopped and
         * the service is ready to start a new data capture. If this method is
         * called when the service is already stopped (i.e. not running) this
         * function returns without any action taken.
         */
        ["ami"] void stop();
        
        /**
         * Returns the state of the data capture service, which includes the
         * ID of the scheduling block for which data capture is currently
         * in progress.
         *
         * @return The scheduling block ID for which data capture is currently
         *          occuring, or -1 if data capture is not running.
         */
        ["ami"] long getState();
    };

};
};
};

#endif
