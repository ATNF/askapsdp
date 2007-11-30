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
    /// @brief Support for parallel algorithms implementing imaging
    ///
    /// @details Provides imaging using the measurement equation framework.
    ///
    /// The control parameters are specified in a parset file. For example:
    /// @code
    ///  	Cimager.dataset                 = [data/spw_1/sim.ms]
    ///  	#Feed                           = 5
    ///
    ///  	Cimager.Images.Names                    = [image.i.sim]
    ///  	Cimager.Images.image.i.sim.shape                = [4096,4096]
    ///  	Cimager.Images.image.i.sim.cellsize     = [6arcsec, 6arcsec]
    ///  	Cimager.Images.image.i.sim.frequency    = [1.164e9, 1.420e9]
    ///  	Cimager.Images.image.i.sim.nchan                = 1
    ///  	Cimager.Images.image.i.sim.direction    = [12h30m00.00, -45.00.00.00, J2000]
    ///
    ///  	#Cimager.gridder                          = WProject
    ///  	Cimager.gridder.WProject.wmax            = 8000
    ///  	Cimager.gridder.WProject.nwplanes        = 64
    ///  	Cimager.gridder.WProject.oversample     = 1
    ///  	Cimager.gridder.WProject.cutoff         = 0.001
    ///
    ///  	Cimager.gridder                                 = AntennaIllum
    ///  	Cimager.gridder.AntennaIllum.wmax               = 8000
    ///  	Cimager.gridder.AntennaIllum.nwplanes           = 64
    ///  	Cimager.gridder.AntennaIllum.oversample         = 1
    ///  	Cimager.gridder.AntennaIllum.diameter           = 12m
    ///  	Cimager.gridder.AntennaIllum.blockage           = 1m
    ///  	Cimager.gridder.AntennaIllum.maxfeeds           = 18
    ///  	Cimager.gridder.AntennaIllum.maxsupport         = 256
    ///
    ///  	Cimager.solver                                  = Dirty
    ///  	Cimager.solver.Dirty.cycles                     = 1
    ///  	Cimager.solver.Dirty.threshold                  = 10%
    ///  	Cimager.solver.Dirty.verbose                    = True
    ///
    ///  	Cimager.solver.Clean.algorithm                  = MultiScale
    ///  	Cimager.solver.Clean.niter                      = 1000
    ///  	Cimager.solver.Clean.gain                       = 0.7
    ///  	Cimager.solver.Clean.cycles                     = 5
    ///  	Cimager.solver.Clean.verbose                    = True
    ///
    ///  	Cimager.restore                                 = True
    ///  	Cimager.restore.beam                            = [30arcsec, 30arcsec, 0deg]
    /// @endcode
    /// @ingroup parallel
    class ImagerParallel : public MEParallel
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
      ImagerParallel(int argc, const char** argv,
          const LOFAR::ACC::APS::ParameterSet& parset);

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
    /// @param discard Discard old equation?
      void calcOne(const string& dataset, bool discard=true);

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
