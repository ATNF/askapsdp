/// @file
///
/// ImageSolverFactory: Factory class for image solvers
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_IMAGESOLVERFACTORY_H_
#define CONRAD_SYNTHESIS_IMAGESOLVERFACTORY_H_

#include <fitting/Solver.h>
#include <fitting/Params.h>

#include <APS/ParameterSet.h>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Construct image solvers according to parameters
    /// @ingroup measurementequation
    class ImageSolverFactory
    {
      public:
        ImageSolverFactory();
        
        ~ImageSolverFactory();

        /// @brief Make a shared pointer for an image solver
        /// @param ip Params for the solver
        /// @param parset ParameterSet containing description of
        /// solver to be constructed
        static conrad::scimath::Solver::ShPtr make(conrad::scimath::Params& ip, 
          const LOFAR::ACC::APS::ParameterSet& parset); 
    };

  }
}
#endif
