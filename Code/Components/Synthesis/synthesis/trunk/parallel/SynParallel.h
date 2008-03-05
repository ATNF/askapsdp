/// @file
///
/// Provides generic methods for parallel algorithms
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef ASKAP_SYNTHESIS_SYNPARALLEL_H_
#define ASKAP_SYNTHESIS_SYNPARALLEL_H_

#include <fitting/Equation.h>
#include <fitting/Solver.h>
#include <fitting/INormalEquations.h>
#include <fitting/Params.h>

#include <askapparallel/AskapParallel.h>

namespace askap
{
  namespace synthesis
  {
    /// @brief Support for parallel algorithms 
    ///
    /// @details Support for parallel applications in the area.
    /// An application is derived from this abstract base. The model used is that the
    /// application has many workers and one master, running in separate MPI processes
    /// or in one single thread. The master is the master so the number of processes
    /// is one more than the number of workers. 
    ///
    /// If the number of nodes is 1 then everything occurs in the same process with
    /// no overall for transmission of model.
    ///
    /// @ingroup parallel
    class SynParallel : public askap::cp::AskapParallel
    {
  public:

      /// @brief Constructor 
      /// @details The command line inputs are needed solely for MPI - currently no
      /// application specific information is passed on the command line.
      /// @param argc Number of command line inputs
      /// @param argv Command line inputs
      SynParallel(int argc, const char** argv);

      ~SynParallel();

      /// Return the model
      askap::scimath::Params::ShPtr& params();

      /// @brief Broadcast the model to all workers
      void broadcastModel();

      /// @brief Receive the model from the master
      void receiveModel();

  protected:

      /// The model
      askap::scimath::Params::ShPtr itsModel;
    };

  }
}
#endif
