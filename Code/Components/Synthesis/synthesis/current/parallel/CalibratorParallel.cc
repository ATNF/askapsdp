/// @file
///
/// CalibratorParallel: Support for parallel applications using the measurement 
/// equation classes. This code applies to calibration. I expect that this part
/// will be redesigned in the future for a better separation of the algorithm
/// from the parallel framework middleware. Current version is basically an
/// adapted ImagerParallel clas
///
/// Performs calibration on a data source. Can run in serial or 
/// parallel (MPI) mode.
///
/// The data are accessed from the DataSource. This is and will probably remain
/// disk based. The images are kept purely in memory until the end.
///
/// Control parameters are passed in from a LOFAR ParameterSet file.
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

// Include own header file first
#include <parallel/CalibratorParallel.h>

// System includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// logging stuff
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallel");

// own includes
#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>

#include <fitting/LinearSolver.h>
#include <fitting/GenericNormalEquations.h>
#include <fitting/Params.h>

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/MEParsetInterface.h>
#include <measurementequation/CalibrationME.h>
#include <measurementequation/ComponentEquation.h>
#include <measurementequation/NoXPolGain.h>
#include <measurementequation/LeakageTerm.h>
#include <measurementequation/Product.h>
#include <measurementequation/ImagingEquationAdapter.h>
#include <gridding/VisGridderFactory.h>
#include <askapparallel/AskapParallel.h>
#include <Common/ParameterSet.h>

// casa includes
#include <casa/aips.h>
#include <casa/OS/Timer.h>

using namespace askap;
using namespace askap::scimath;
using namespace askap::synthesis;
using namespace askap::mwbase;

/// @brief Constructor from ParameterSet
/// @details The parset is used to construct the internal state. We could
/// also support construction from a python dictionary (for example).
/// The command line inputs are needed solely for MPI - currently no
/// application specific information is passed on the command line.
/// @param[in] parset ParameterSet for inputs
CalibratorParallel::CalibratorParallel(askap::mwbase::AskapParallel& comms,
        const LOFAR::ParameterSet& parset) :
      MEParallelApp(comms,parset), 
      itsPerfectModel(new scimath::Params()), itsSolveGains(false), itsSolveLeakage(false)
{
  // set up image handler, needed for both master and worker
  SynthesisParamsHelper::setUpImageHandler(parset);
  
  const std::string what2solve = parset.getString("solve","gains");
  if (what2solve.find("gains") != std::string::npos) {
      ASKAPLOG_INFO_STR(logger, "Gains will be solved for (solve='"<<what2solve<<"')");
      itsSolveGains = true;
  }
  if (what2solve.find("leakages") != std::string::npos) {
      ASKAPLOG_INFO_STR(logger, "Leakages will be solved for (solve='"<<what2solve<<"')");
      itsSolveLeakage = true;
  }
  ASKAPCHECK(itsSolveGains || itsSolveLeakage, 
      "Nothing to solve! Either gains or leakages (or both) have to be solved for, you specified solve='"<<
      what2solve<<"'");
  
  if (itsComms.isMaster()) {
      
      // itsModel has gain parameters for calibration, populate them with
      // an initial guess
      ASKAPDEBUGASSERT(itsModel); // should be initialized in SynParallel
      
      
      // initial assumption of the parameters
      const casa::uInt nAnt = parset.getInt32("nAnt",36); // 28  
      const casa::uInt nBeam = parset.getInt32("nBeam",1); 
      if (itsSolveGains) {
          ASKAPLOG_INFO_STR(logger, "Initialise gains (unknowns) for "<<nAnt<<" antennas and "<<nBeam<<" beam(s).");
          for (casa::uInt ant = 0; ant<nAnt; ++ant) {
               for (casa::uInt beam = 0; beam<nBeam; ++beam) {
                    itsModel->add("gain.g11."+utility::toString(ant)+"."+utility::toString(beam),casa::Complex(1.,0.));
                    itsModel->add("gain.g22."+utility::toString(ant)+"."+utility::toString(beam),casa::Complex(1.,0.));
                    //itsModel->fix("gain.g11."+utility::toString(ant));
               }
          }
      }
      if (itsSolveLeakage) {
          ASKAPLOG_INFO_STR(logger, "Initialise leakages (unknowns) for "<<nAnt<<" antennas and "<<nBeam<<" beam(s).");
          for (casa::uInt ant = 0; ant<nAnt; ++ant) {
               for (casa::uInt beam = 0; beam<nBeam; ++beam) {
                    itsModel->add("leakage.d12."+utility::toString(ant)+"."+utility::toString(beam),casa::Complex(0.,0.));
                    itsModel->add("leakage.d21."+utility::toString(ant)+"."+utility::toString(beam),casa::Complex(0.,0.));
               }
          }
      }
      
      
      /// Create the solver  
      itsSolver.reset(new LinearSolver);
      ASKAPCHECK(itsSolver, "Solver not defined correctly");
      itsRefGain = parset.getString("refgain","");
  }
  if (itsComms.isWorker()) {
      // load sky model, populate itsPerfectModel
      readModels();
  }
}

/// @brief read the model from parset file and populate itsPerfectModel
/// @details This method is common between several classes and probably
/// should be pushed up in the class hierarchy
void CalibratorParallel::readModels()
{
  LOFAR::ParameterSet parset(MEParallelApp::parset());
  if (MEParallelApp::parset().isDefined("sources.definition")) {
    parset = LOFAR::ParameterSet(substitute(MEParallelApp::parset().getString("sources.definition")));
  }
      
  const std::vector<std::string> sources = parset.getStringVector("sources.names");
  for (size_t i=0; i<sources.size(); ++i) {
	   const std::string modelPar = std::string("sources.")+sources[i]+".model";
	   
	   if (parset.isDefined(modelPar)) {
           const std::string model=parset.getString(modelPar);
           ASKAPLOG_INFO_STR(logger, "Adding image " << model << " as model for "<< sources[i] );
           const std::string paramName = "image.i."+sources[i];
           SynthesisParamsHelper::loadImageParameter(*itsPerfectModel, paramName, model);
       } else {
          // this is an individual component, rather then a model defined by image
          ASKAPLOG_INFO_STR(logger, "Adding component description as model for "<< sources[i] );
          SynthesisParamsHelper::copyComponent(itsPerfectModel, parset,sources[i],"sources.");
       }
  }
  ASKAPLOG_INFO_STR(logger, "Successfully read models");
}

void CalibratorParallel::calcOne(const std::string& ms, bool discard)
{
  casa::Timer timer;
  timer.mark();
  ASKAPLOG_INFO_STR(logger, "Calculating normal equations for " << ms );
  // First time around we need to generate the equation 
  if ((!itsEquation) || discard) {
      ASKAPLOG_INFO_STR(logger, "Creating measurement equation" );
      TableDataSource ds(ms, TableDataSource::DEFAULT, dataColumn());
      ds.configureUVWMachineCache(uvwMachineCacheSize(),uvwMachineCacheTolerance());      
      IDataSelectorPtr sel=ds.createSelector();
      sel << parset();
      IDataConverterPtr conv=ds.createConverter();
      conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
          "Hz");
      conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
      IDataSharedIter it=ds.createIterator(sel, conv);
      ASKAPCHECK(itsPerfectModel, "Uncorrupted model not defined");
      ASKAPCHECK(itsModel, "Initial assumption of parameters is not defined");
      ASKAPCHECK(gridder(), "Gridder not defined");
      if (SynthesisParamsHelper::hasImage(itsPerfectModel)) {
         ASKAPCHECK(!SynthesisParamsHelper::hasComponent(itsPerfectModel),
                 "Image + component case has not yet been implemented");
         // have to create an image-specific equation        
         boost::shared_ptr<ImagingEquationAdapter> ieAdapter(new ImagingEquationAdapter);
         ieAdapter->assign<ImageFFTEquation>(*itsPerfectModel, gridder());
         createCalibrationME(it,ieAdapter);
      } else {
         // model is a number of components, don't need an adapter here
         
         // it doesn't matter which iterator is passed below. It is not used
         boost::shared_ptr<ComponentEquation> 
                  compEq(new ComponentEquation(*itsPerfectModel,it));
         createCalibrationME(it,compEq);         
      }
  } else {
      ASKAPLOG_INFO_STR(logger, "Reusing measurement equation" );
  }
  ASKAPCHECK(itsEquation, "Equation not defined");
  ASKAPCHECK(itsNe, "NormalEquations not defined");
  itsEquation->calcEquations(*itsNe);
  ASKAPLOG_INFO_STR(logger, "Calculated normal equations for "<< ms << " in "<< timer.real()
                     << " seconds ");
}

/// @brief create measurement equation
/// @details This method initialises itsEquation with shared pointer to a proper type.
/// It uses internal flags to create a correct type (i.e. polarisation calibration or
/// just antenna-based gains). Parameters are passed directly to the constructor of 
/// CalibrationME template.
/// @param[in] dsi data shared iterator 
/// @param[in] perfectME uncorrupted measurement equation
void CalibratorParallel::createCalibrationME(const IDataSharedIter &dsi, 
                const boost::shared_ptr<IMeasurementEquation const> &perfectME)
{
   ASKAPDEBUGASSERT(itsModel);
   if (itsSolveGains && !itsSolveLeakage) {
       itsEquation.reset(new CalibrationME<NoXPolGain>(*itsModel,dsi,perfectME));           
   } else if (itsSolveLeakage && !itsSolveGains) {
       itsEquation.reset(new CalibrationME<LeakageTerm>(*itsModel,dsi,perfectME));           
   } else if (itsSolveLeakage && itsSolveGains) {
       itsEquation.reset(new CalibrationME<Product<NoXPolGain,LeakageTerm> >(*itsModel,dsi,perfectME));           
   } else {
       ASKAPTHROW(AskapError, "Unsupported combination of itsSolveGains and itsSolveLeakage. This shouldn't happen. Verify solve parameter");       
   }
}

/// Calculate the normal equations for a given measurement set
void CalibratorParallel::calcNE()
{
  /// Now we need to recreate the normal equations
  itsNe.reset(new GenericNormalEquations);

  if (itsComms.isWorker()) {
        
      ASKAPDEBUGASSERT(itsNe);

      if (itsComms.isParallel()) {
          calcOne(measurementSets()[itsComms.rank()-1]);
          sendNE();
      } else {
          ASKAPCHECK(itsSolver, "Solver not defined correctly");
          itsSolver->init();
          for (size_t iMs=0; iMs<measurementSets().size(); ++iMs) {
            calcOne(measurementSets()[iMs]);
            itsSolver->addNormalEquations(*itsNe);
          }
      }
  }
}

void CalibratorParallel::solveNE()
{ 
  if (itsComms.isMaster()) {
      // Receive the normal equations
      if (itsComms.isParallel()) {
          receiveNE();
      }
        
      ASKAPLOG_INFO_STR(logger, "Solving normal equations");
      casa::Timer timer;
      timer.mark();
      Quality q;
      ASKAPDEBUGASSERT(itsSolver);
      itsSolver->setAlgorithm("SVD");     
      itsSolver->solveNormalEquations(*itsModel,q);
      ASKAPLOG_INFO_STR(logger, "Solved normal equations in "<< timer.real() << " seconds ");
      if (itsRefGain != "") {
          ASKAPLOG_INFO_STR(logger, "Rotating phases to have that of "<<itsRefGain<<" equal to 0");
          rotatePhases();
      }
  }
}

/// @brief helper method to rotate all phases
/// @details This method rotates the phases of all gains in itsModel
/// to have the phase of itsRefGain exactly 0. This operation does
/// not seem to be necessary for SVD solvers, however it simplifies
/// "human eye" analysis of the results (otherwise the phase degeneracy
/// would make the solution different from the simulated gains).
/// @note The method throws exception if itsRefGain is not among
/// the parameters of itsModel
void CalibratorParallel::rotatePhases()
{
  ASKAPDEBUGASSERT(itsComms.isMaster());
  ASKAPDEBUGASSERT(itsModel);
  ASKAPCHECK(itsModel->has(itsRefGain), "phase rotation to `"<<itsRefGain<<
              "` is impossible because this parameter is not present in the model");
  
  const casa::Complex refPhaseTerm = casa::polar(1.f,
                -arg(itsModel->complexValue(itsRefGain)));
                       
  std::vector<std::string> names(itsModel->freeNames());
  for (std::vector<std::string>::const_iterator it=names.begin();
               it!=names.end();++it)  {
       const std::string parname = *it;
       if (parname.find("gain") != std::string::npos) {                    
           itsModel->update(parname,
                 itsModel->complexValue(parname)*refPhaseTerm);                                 
       } 
  }             
}

/// @brief Write the results (runs in the solver)
/// @details The solution (calibration parameters) is written into 
/// an external file in the parset file format.
/// @param[in] postfix a string to be added to the file name
void CalibratorParallel::writeModel(const std::string &postfix)
{
  if (itsComms.isMaster()) {
      ASKAPLOG_INFO_STR(logger, "Writing out results into a parset file");
      ASKAPCHECK(postfix == "", "postfix parameter is not supposed to be used in the calibration code");
      
      ASKAPDEBUGASSERT(itsModel);
      std::vector<std::string> parlist = itsModel->names();
      std::ofstream os("result.dat"); // for now just hard code it
      for (std::vector<std::string>::const_iterator it = parlist.begin(); 
           it != parlist.end(); ++it) {
           const casa::Complex gain = itsModel->complexValue(*it);
           os<<*it<<" = ["<<real(gain)<<","<<imag(gain)<<"]"<<std::endl;
      }
      /*
      // temporary for debugging/research
      std::ofstream os2("deviation.dat"); // for now just hard code it
      LOFAR::ParameterSet trueGains("rndgains.in");
      scimath::Params par;
      par<<trueGains;
      for (std::vector<std::string>::const_iterator it = parlist.begin(); 
           it != parlist.end(); ++it) {
           const casa::Complex gain = itsModel->complexValue(*it);
           const casa::Complex diff = gain - par.complexValue(*it);
           os2<<*it<<" "<<real(diff)<<" "<<imag(diff)<<" "<<abs(diff)<<" "<<arg(diff)/M_PI*180.<<std::endl;
      }
      */
  }
}

