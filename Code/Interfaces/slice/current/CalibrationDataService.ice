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

#include <CalibrationParameters.ice>

module askap
{
module interfaces
{
module caldataservice
{
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
         */
        void addGainsSolution(askap::interfaces::calparams::TimeTaggedGainSolution solution);

        /**
         * Add a new time tagged bandpass solution to the calibration
         * data service.
         * 
         * @param solution  new time tagged bandpass solution.
         */
        void addBandpassSolution(askap::interfaces::calparams::TimeTaggedBandpassSolution solution);

        /**
         * Add a new time tagged leakage solution to the calibration
         * data service.
         * 
         * @param solution  new time tagged leakage solution.
         */
        void addLeakageSolution(askap::interfaces::calparams::TimeTaggedLeakageSolution solution);

        /**
         * Obtains the most recent calibration solution. The ingest pipeline is
         * expected to call this periodically to obtain the latest solution.
         *
         * @return The most recent calibration solution.
         */
         askap::interfaces::calparams::CalibrationParametersMap getCurrentSolution();
    };

};
};
};

#endif
