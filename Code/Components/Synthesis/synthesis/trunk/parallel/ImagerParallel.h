/// @file
///
/// ImagerParallel: Support for parallel applications using the measurement equation
/// classes.
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_IMAGERPARALLEL_H_
#define CONRAD_SYNTHESIS_IMAGERPARALLEL_H_

#include <parallel/MEParallel.h>

#include <gridding/IVisGridder.h>

#include <fitting/Solver.h>

#include <APS/ParameterSet.h>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Support for parallel algorithms
    ///
    /// @details Provides generic methods for parallel algorithms
    /// @ingroup parallel
    class ImagerParallel: public MEParallel
    {
      public:

        /// @brief Constructor from ParameterSet
      	/// @details The parset is used to construct the internal state. We could
      	/// also support construction from a python dictionary (for example).
      	/// The command line inputs are needed solely for MPI - currently no
      	/// application specific information is passed on the command line.
      	/// @param argc Number of command line inputs
      	/// @param argv Command line inputs
      	/// @param parset ParameterSet for inputs
      	ImagerParallel(int argc, const char** argv, const LOFAR::ACC::APS::ParameterSet& parset);
      	
      	/// @brief Calculate the normalequations (runs in the prediffers)
      	/// @details ImageFFTEquation and the specified gridder (set in the parset
      	/// file) are used to calculate the normal equations. The image parameters
      	/// are defined in the parset file.
      	virtual void calcNE();
      	
      	/// @brief Solve the normal equations (runs in the solver)
      	/// @details Either a dirty image can be constructed or the 
      	/// multiscale clean can be used, as specified in the parset file.
      	virtual void solveNE();

      	/// @brief Write the results (runs in the solver)
      	/// @details The model images are written as AIPS++ images. In addition,
      	/// the images may be restored using the specified beam.
      	virtual void writeModel();

      private:
      	
      	/// Calculate normal equations for one data set
      	/// @param ms Name of data set
      	void calcOne(const string& dataset);
      	
      	/// ParameterSet
      	LOFAR::ACC::APS::ParameterSet itsParset;
      	
      	/// Do we want a restored image?
    		bool itsRestore; 
    		
    		/// Names of measurement sets, one per prediffer
    		vector<string> itsMs; 
    		
    		/// Restoring beam
    		casa::Vector<casa::Quantum<double> > itsQbeam; 

    		/// Gridder to be used
    		IVisGridder::ShPtr itsGridder; 
    		
    };

  }
}
#endif
