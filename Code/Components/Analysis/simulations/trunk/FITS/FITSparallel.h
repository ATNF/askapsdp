/// @file
///
/// A class to control parallel operation of FITS creation
///
/// @copyright (c) 2008 CSIRO
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
#ifndef ASKAP_SIMULATIONS_FITSPARALLEL_H_
#define ASKAP_SIMULATIONS_FITSPARALLEL_H_

#include <askapparallel/AskapParallel.h>

#include <Common/ParameterSet.h>
#include <duchamp/Utils/Section.hh>
#include <analysisutilities/SubimageDef.h>

#include <FITS/FITSfile.h>

#include <vector>

namespace askap {
    namespace simulations {
        namespace FITS {

            /// @brief Support for parallel FITS creation
            ///
            /// @details This class manages the creation of FITS files in
            /// a parallel environment.
            /// The model used is that the application has many workers and
            /// one master, running in separate MPI processes or in one single
            /// thread. The master is the master so the number of processes is one
            /// more than the number of workers.
            ///
            /// If the number of nodes is 1 then everything occurs in the same process.
            ///
            class FITSparallel : public askap::mwbase::AskapParallel {
                public:

                    /// @brief Constructor
                    /// @details The command line inputs are needed solely for MPI - currently no
                    /// application specific information is passed on the command line.
                    /// @param argc Number of command line inputs
                    /// @param argv Command line inputs
                    /// @param parset The parameter set to read Duchamp and other parameters from.
                    FITSparallel(int argc, const char** argv, const LOFAR::ParameterSet& parset);

                    /// @brief Destructor
                    virtual ~FITSparallel() {};

                    /// @brief Send the array to the Master node
                    void toMaster();

                    /// @brief Add noise to the flux array
                    void addNoise();

                    /// @brief Add sources to the flux array
                    void processSources();

                    /// @brief Convolve the flux array with a beam
                    void convolveWithBeam();

                    /// @brief Save the array to a FITS file
                    void saveFile();


                protected:

                    /// @brief The FITS file class
                    FITSfile itsFITSfile;

                    /// @brief The subimage definition
                    analysis::SubimageDef itsSubimageDef;
                    /// @brief The subsection being used
                    duchamp::Section itsSubsection;

            };

        }
    }
}

#endif

