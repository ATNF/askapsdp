// @file CalibrationDataService.ice
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

#ifndef ASKAP_CALIBRATION_DATA_SERVICE_ICE
#define ASKAP_CALIBRATION_DATA_SERVICE_ICE

#include <CommonTypes.ice>
#include <CalibrationParameters.ice>

module askap
{
module interfaces
{
module caldataservice
{
    /**
     * This exception is thrown when a solution id is specified but does not exist,
     * when is expected to.
     **/
    exception UnknownSolutionIdException extends askap::interfaces::AskapIceException
    {
    };

    /**
     * Interface to the Calibration Data Service.
     **/
    interface ICalibrationDataService
    {
        /**
         * Add a new time tagged gains solution to the calibration
         * data service.
         * 
         * @param solution  new time tagged gains solution.
         * @return the unique ID for the added solution.
         */
        long addGainsSolution(askap::interfaces::calparams::TimeTaggedGainSolution solution);

        /**
         * Add a new time tagged bandpass solution to the calibration
         * data service.
         * 
         * @param solution  new time tagged bandpass solution.
         * @return the unique ID for the added solution.
         */
        long addBandpassSolution(askap::interfaces::calparams::TimeTaggedBandpassSolution solution);

        /**
         * Add a new time tagged leakage solution to the calibration
         * data service.
         * 
         * @param solution  new time tagged leakage solution.
         * @return the unique ID for the added solution.
         */
        long addLeakageSolution(askap::interfaces::calparams::TimeTaggedLeakageSolution solution);

        /**
         * Obtain the ID of the current/optimum gain solution.
         * @return the ID of the latest/optimum gain solution.
         */
        long getCurrentGainSolutionID();

        /**
         * Obtain the ID of the current/optimum leakage solution.
         * @return the ID of the latest/optimum leakage solution.
         */
        long getCurrentLeakageSolutionID();

        /**
         * Obtain the ID of the current/optimum bandpass solution.
         * @return the ID of the latest/optimum bandpass solution.
         */
        long getCurrentBandpassSolutionID();

        /**
         * Get a gain solution.
         * @param id    id of the gain solution to obtain.
         * @return the gain solution.
         *
         * @throws UnknownSolutionIdException   the id parameter does not refer to
         *                                      a known solution.
         */
        askap::interfaces::calparams::TimeTaggedGainSolution getGainSolution(long id)
            throws UnknownSolutionIdException;

        /**
         * Get a leakage solution.
         * @param id    id of the leakage solution to obtain.
         * @return the leakage solution.
         *
         * @throws UnknownSolutionIdException   the id parameter does not refer to
         *                                      a known solution.
         */
        askap::interfaces::calparams::TimeTaggedLeakageSolution getLeakageSolution(long id)
            throws UnknownSolutionIdException;

        /**
         * Get a bandpass solution.
         * @param id    id of the bandpass solution to obtain.
         * @return the bandpass solution.
         *
         * @throws UnknownSolutionIdException   the id parameter does not refer to
         *                                      a known solution.
         */
        askap::interfaces::calparams::TimeTaggedBandpassSolution getBandpassSolution(long id)
            throws UnknownSolutionIdException;
    };

};
};
};

#endif
