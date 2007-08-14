/// @file
///
/// SynParallel: Support for parallel applications
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_IMAGERPARALLEL_H_
#define CONRAD_SYNTHESIS_IMAGERPARALLEL_H_

#include <parallel/SynParallel.h>

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
    class ImagerParallel: public SynParallel
    {
      public:

        /// @brief Constructor from ParameterSet
      	/// @param argc Number of command line inputs
      	/// @param argv Command line inputs
      	/// @param parset ParameterSet for inputs
      	ImagerParallel(int argc, const char** argv, const LOFAR::ACC::APS::ParameterSet& parset);
      	
      	/// Calculate the normalequations
      	virtual void calcNE();
      	
      	/// Solve the normal equations
      	virtual void solveNE();

      	/// Write the results
      	virtual void writeModel();

      private:
      	
      	/// Calculate normal equations for one MS
      	void calcOne(const string& ms);
      	
      	/// ParameterSet
      	LOFAR::ACC::APS::ParameterSet itsParset;
      	
      	/// Do we want a restored image?
    		bool itsRestore; 
    		
    		/// Names of measurement sets
    		vector<string> itsMs; 
    		
    		/// Restoring beam
    		casa::Vector<casa::Quantum<double> > itsQbeam; 

    		/// Gridder to be used
    		IVisGridder::ShPtr itsGridder; 
    		
    };

  }
}
#endif
