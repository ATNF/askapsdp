/// @file
///
/// CalibratorParallel: Support for parallel applications using the measurement 
/// equation classes. This code applies to calibration. I expect that this part
/// will be redesigned in the future for a better separation of the algorithm
/// from the parallel framework middleware. Current version is basically an
/// adapted ImagerParallel clas
///
/// @copyright (c) 2007 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///
#ifndef CALIBRATOR_PARALLEL_H
#define CALIBRATOR_PARALLEL_H

// own includes
#include <askapparallel/AskapParallel.h>
#include <parallel/MEParallel.h>
#include <Common/ParameterSet.h>
#include <gridding/IVisGridder.h>
#include <measurementequation/IMeasurementEquation.h>
#include <dataaccess/SharedIter.h>

// std includes
#include <string>
#include <vector>

// boost includes
#include <boost/shared_ptr.hpp>
#include <fitting/Solver.h>

namespace askap
{
  namespace synthesis
  {
    /// @brief Support for parallel algorithms implementing calibration
    ///
    /// @details Provides calibration using the measurement equation framework.
    ///
    /// The control parameters are specified in a parset file. For example:
    /// @code
    ///  	Ccalibrator.datacolumnset           = DATACOL     # default is DATA
    ///  	Ccalibrator.dataset                 = [data/spw_1/sim.ms]
    ///  	#Feed                           = 5
    ///
    ///  	Ccalibrator.sources.names                =       [10uJy]
    ///  	Ccalibrator.sources.10uJy.direction       =       [12h30m00.000, -45.00.00.000, J2000]
    ///  	Ccalibrator.sources.10uJy.model   =       10uJy.model
    ///
    ///  	Ccalibrator.gridder                          = WProject
    ///  	Ccalibrator.gridder.WProject.wmax            = 8000
    ///  	Ccalibrator.gridder.WProject.nwplanes        = 64
    ///  	Ccalibrator.gridder.WProject.oversample     = 1
    ///  	Ccalibrator.gridder.WProject.cutoff         = 0.001
    ///
    /// @endcode
    /// @ingroup parallel
    class CalibratorParallel : public MEParallel
    {
  public:

      /// @brief Constructor from ParameterSet
      /// @details The parset is used to construct the internal state. We could
      /// also support construction from a python dictionary (for example).
      /// The command line inputs are needed solely for MPI - currently no
      /// application specific information is passed on the command line.
      /// @param[in] comms communication object
      /// @param[in] parset ParameterSet for inputs
      CalibratorParallel(askap::mwbase::AskapParallel& comms,
          const LOFAR::ParameterSet& parset);

      /// @brief Calculate the normal equations (runs in the prediffers)
      /// @details ImageFFTEquation and the specified gridder (set in the parset
      /// file) are used in conjunction with CalibrationME to calculate 
      /// the generic normal equations. The image parameters used in the uncorrupted
      /// measurement equation are defined in the parset file.
      virtual void calcNE();

      /// @brief Solve the normal equations (runs in the solver)
      /// @details Parameters of the calibration problem are solved for here
      virtual void solveNE();

      /// @brief Write the results (runs in the solver)
      /// @details The solution (calibration parameters) is written into 
      /// an external file in the parset file format.
      /// @param[in] postfix a string to be added to the file name
	  virtual void writeModel(const std::string &postfix = std::string());
      

  protected:      
  
      /// @brief create measurement equation
      /// @details This method initialises itsEquation with shared pointer to a proper type.
      /// It uses internal flags to create a correct type (i.e. polarisation calibration or
      /// just antenna-based gains). Parameters are passed directly to the constructor of 
      /// CalibrationME template.
      /// @param[in] dsi data shared iterator 
      /// @param[in] perfectME uncorrupted measurement equation
      void createCalibrationME(const IDataSharedIter &dsi, 
                const boost::shared_ptr<IMeasurementEquation const> &perfectME);
  
      /// @brief helper method to rotate all phases
      /// @details This method rotates the phases of all gains in itsModel
      /// to have the phase of itsRefGain exactly 0. This operation does
      /// not seem to be necessary for SVD solvers, however it simplifies
      /// "human eye" analysis of the results (otherwise the phase degeneracy
      /// would make the solution different from the simulated gains).
      /// @note The method throws exception if itsRefGain is not among
      /// the parameters of itsModel
      void rotatePhases();
      
  private:
      /// @brief read the model from parset file and populate itsPerfectModel
      /// @details This method is common between several classes and probably
      /// should be pushed up in the class hierarchy
      void readModels();
 
      /// Calculate normal equations for one data set
      /// @param[in] ms Name of data set
      /// @param[in] discard Discard old equation?
      void calcOne(const std::string& dataset, bool discard=true);

      /// ParameterSet
      LOFAR::ParameterSet itsParset;

      /// Name of data column to use.
      std::string itsColName;

      /// Names of measurement sets, one per prediffer
      std::vector<std::string> itsMs;

      /// Gridder to be used
      IVisGridder::ShPtr itsGridder;
      
      /// uncorrupted model
      askap::scimath::Params::ShPtr itsPerfectModel;
      
      /// @brief name of the parameter taken as a reference
      /// @details empty string means no referencing is required
      std::string itsRefGain;
      
      /// @brief flag switching the gain calibration on
      bool itsSolveGains;
      
      /// @brief flag swtiching the leakage calibration on
      bool itsSolveLeakage;
    };

  }
}
#endif // #ifndef CALIBRATOR_PARALLEL_H

