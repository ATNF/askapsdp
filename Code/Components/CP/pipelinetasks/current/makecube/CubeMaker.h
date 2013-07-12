/// @file CubeMaker.h
///
/// Class to run the creation of a new cube
///
/// @copyright (c) 2013 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#ifndef ASKAP_CP_PIPELINETASKS_CUBEMAKER_H
#define ASKAP_CP_PIPELINETASKS_CUBEMAKER_H

// System includes
#include <vector>
#include <string>

// ASKAPsoft includes
#include <Common/ParameterSet.h>
#include <boost/scoped_ptr.hpp>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Unit.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/PagedImage.h>

namespace askap {
namespace cp {
namespace pipelinetasks {

/// @brief Class to handle functionality for the makecube application.
///
/// @details This class handles most of the aspects of the combination of
/// individual channel images into a spectral cube. It allows ParameterSet
/// specification of input and output parameters, the rest frequency (if
/// needed), and the recording of the beam shapes for the individual channel
/// images.
class CubeMaker {
    public:
        /// Constructor
        CubeMaker(const LOFAR::ParameterSet& parset);

        /// Destructor
        virtual ~CubeMaker();

        /// @brief Set up the list of input files and the reference data
        void initialise();

        /// @brief Create the output cube
        void createCube();

        /// @brief Set the units and beam for the output cube
        void setImageInfo();

        /// @brief Write the individual channel images to the output cube
        void writeSlices();

        /// @brief Record the beams of the input images
        void recordBeams();

        /// @brief Rest frequency of the HI fine-structure line [Hz]
        static const double REST_FREQ_HI = 1420405751.786;

    private:
        /// @brief Read the reference coordinate system data
        void getReferenceData();

        /// @brief Write the rest frequency to a coordinate system
        void setRestFreq(casa::CoordinateSystem& csys);

        /// @brief Write an individual channel image to the cube
        bool writeSlice(size_t i);

        const std::string itsInputNamePattern;
        const std::string itsCubeName;
        const std::string itsBeamReference;
        const std::string itsBeamFile;

        std::vector<std::string> itsInputNames;
        double itsRestFrequency;

        size_t itsBeamImageNum;

        int itsNumChan;
        casa::IPosition itsRefShape;
        casa::CoordinateSystem itsRefCoordinates;
        casa::CoordinateSystem itsSecondCoordinates;
        casa::Unit itsRefUnits;

        boost::scoped_ptr< casa::PagedImage<float> > itsCube;
};

}
}
}

#endif
