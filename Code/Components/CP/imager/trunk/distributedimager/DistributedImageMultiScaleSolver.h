/// @file
///
/// DistributedImageMultiScaleSolver: This solver calculates the dirty image (or equivalent)
/// for all parameters called image*
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CP_DISTRIBUTEDIMAGEMULTISCALESOLVER_H_
#define CP_DISTRIBUTEDIMAGEMULTISCALESOLVER_H_

// System includes
#include <map>

// Boost includes
#include <boost/shared_ptr.hpp>

// ASKAPsoft includes
#include <APS/ParameterSet.h>
#include <measurementequation/ImageCleaningSolver.h>
#include <lattices/Lattices/LatticeCleaner.h>

// Local includes
#include <distributedimager/IBasicComms.h>

namespace askap
{
    namespace cp
    {
        /// @brief Multiscale solver for images.
        ///
        /// @details This solver performs multi-scale clean using the 
        /// casa::LatticeCleaner classes
        ///
        /// @ingroup measurementequation
        class DistributedImageMultiScaleSolver : public askap::synthesis::ImageCleaningSolver
        {
            public:

                /// @brief Constructor from parameters.
                /// The parameters named image* will be interpreted as images and
                /// solutions formed by the method described.
                /// The default scales are 0, 10, 30 pixels
                /// @param ip Parameters i.e. the images
                DistributedImageMultiScaleSolver(const askap::scimath::Params& ip,
                        const LOFAR::ACC::APS::ParameterSet& parset,
                        askap::cp::IBasicComms& comms);

                /// @brief Constructor from parameters and scales.
                /// The parameters named image* will be interpreted as images and
                /// solutions formed by the method described.
                /// @param ip Parameters i.e. the images
                /// @param scales Scales to be solved in pixels
                DistributedImageMultiScaleSolver(const askap::scimath::Params& ip,
                        const casa::Vector<float>& scales,
                        const LOFAR::ACC::APS::ParameterSet& parset,
                        askap::cp::IBasicComms& comms);

                /// @brief Initialize this solver
                virtual void init();

                /// @brief Solve for parameters, updating the values kept internally
                /// The solution is constructed from the normal equations
                /// @param q Solution quality information
                virtual bool solveNormalEquations(askap::scimath::Quality& q);

                /// @brief Clone this object
                virtual askap::scimath::Solver::ShPtr clone() const;

                /// Set the scales
                void setScales(const casa::Vector<float>& scales);

            protected:
                /// Precondition the PSF and the dirty image
                void preconditionNE(casa::ArrayLattice<float>& psf, casa::ArrayLattice<float>& dirty);

            private:
                /// Scales in pixels
                casa::Vector<float> itsScales;
                /// Map of Cleaners
                std::map<string, boost::shared_ptr<casa::LatticeCleaner<float> > > itsCleaners;

                struct CleanerWork
                {
                    int patchid;
                    boost::shared_ptr< casa::Array<float> > model;
                    bool done;
                    double strengthOptimum;
                };

                // Clean workqueue
                std::vector<CleanerWork> m_cleanworkq;

                void processCleanResponse(void);

                bool outstanding(void);

                // Parameter set
                LOFAR::ACC::APS::ParameterSet m_parset;

                // Communications class
                askap::cp::IBasicComms& m_comms;

        };

    }
}
#endif
