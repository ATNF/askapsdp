/// @file SolverCore.h
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

#ifndef ASKAP_CP_SOLVERCORE_H
#define ASKAP_CP_SOLVERCORE_H

// System includes

// ASKAPsoft includes
#include <Common/ParameterSet.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>
#include <fitting/Solver.h>
#include <fitting/Quality.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Arrays/Vector.h>

// Local includes
#include "distributedimager/common/IBasicComms.h"

namespace askap {
    namespace cp {

        /// @brief Core solver functionality required by the imager.
        class SolverCore
        {
            public:
                /// @brief Constructor
                SolverCore(LOFAR::ParameterSet& parset,
                        askap::cp::IBasicComms& comms,
                        askap::scimath::Params::ShPtr model_p);
                virtual ~SolverCore();

                /// @brief Solve the normal equations (runs in the solver)
                virtual void solveNE(askap::scimath::INormalEquations::ShPtr);

                /// @brief Write the results
                virtual void writeModel(const std::string& postfix);

            private:
                // A helper method to extract peak residual
                double getPeakResidual(askap::scimath::INormalEquations::ShPtr ne_p);

                // Setup the restoring beam parameters (itsQbeam)
                void setupRestoreBeam(void);

                // Solver
                askap::scimath::Solver::ShPtr itsSolver;

                // Parameter set
                LOFAR::ParameterSet& itsParset;
   
                // Communications class
                askap::cp::IBasicComms& itsComms;

                // Model
                askap::scimath::Params::ShPtr itsModel;

                // Restoring beam parameters
                casa::Vector<casa::Quantum<double> > itsQbeam;

                // No support for assignment
                SolverCore& operator=(const SolverCore& rhs);

                // No support for copy constructor
                SolverCore(const SolverCore& src);
        };
    };
};

#endif
