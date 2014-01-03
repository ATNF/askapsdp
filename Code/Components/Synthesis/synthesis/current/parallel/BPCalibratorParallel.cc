/// @file
///
/// BPCalibratorParallel: part of the specialised tool to do optimised bandpass calibration with
/// limited functionality. Unlike CalibratorParallel, this class
///
///      * solves for bandpass only
///      * works only with preaveraging calibration approach
///      * does not support multiple chunks in time (i.e. only one solution is made for the whole dataset)
///      * does not support data distribution except per beam 
///      * does not support a distributed model (e.h. with individual workers dealing with individual Taylor terms)
///      * does not require exact match between number of workers and number of channel chunks, data are dealt with
///        serially by each worker with multiple iterations over data, if required.
///      * solves normal equations at the worker level in the parallel case
///
/// This specialised tool matches closely BETA needs and will be used for BETA initially (at least until we converge
/// on the best approach to do bandpass calibration). The lifetime of this tool is uncertain at present. In many
/// instances the code is quick and dirty, just to suit our immediate needs.  
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
#include <parallel/BPCalibratorParallel.h>

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
#include <measurementequation/PreAvgCalMEBase.h>
#include <measurementequation/ComponentEquation.h>
#include <measurementequation/NoXPolGain.h>
#include <measurementequation/NoXPolFreqDependentGain.h>
#include <measurementequation/NoXPolBeamIndependentGain.h>
#include <measurementequation/ImagingEquationAdapter.h>
#include <gridding/VisGridderFactory.h>
#include <askapparallel/AskapParallel.h>
#include <Common/ParameterSet.h>
#include <calibaccess/CalParamNameHelper.h>
#include <calibaccess/CalibAccessFactory.h>


// casa includes
#include <casa/aips.h>
#include <casa/OS/Timer.h>

namespace askap {

namespace synthesis {

/// @brief Constructor from ParameterSet
/// @details The parset is used to construct the internal state. We could
/// also support construction from a python dictionary (for example).
/// @param[in] comms communication object
/// @param[in] parset ParameterSet for inputs
BPCalibratorParallel::BPCalibratorParallel(askap::askapparallel::AskapParallel& comms,
          const LOFAR::ParameterSet& parset) : MEParallelApp(comms,parset), 
      itsPerfectModel(new scimath::Params()), itsSolutionID(-1), itsSolutionIDValid(false)
{
   ASKAPLOG_INFO_STR(logger, "Bandpass will be solved for using a specialised pipeline");
   if (itsComms.isMaster()) {                        
      // setup solution source (or sink to be exact, because we're writing the solution here)
      itsSolutionSource = accessors::CalibAccessFactory::rwCalSolutionSource(parset);
      ASKAPASSERT(itsSolutionSource);
      
      if (itsComms.isParallel()) {
          ASKAPLOG_INFO_STR(logger, "The work will be distributed between "<<itsComms.nProcs() - 1<<" workers");
      } else {
          ASKAPLOG_INFO_STR(logger, "The work will be done in the serial by the current process");
      }
  }
  if (itsComms.isWorker()) {
      /// Create solver in workers  
      itsSolver.reset(new scimath::LinearSolver);
      ASKAPCHECK(itsSolver, "Solver not defined correctly");
      itsRefGain = parset.getString("refgain","");
      if (itsRefGain.size() > 0) {
          ASKAPLOG_INFO_STR(logger, "Phases will be rotated, so "<<itsRefGain<<" has zero phase for all channels and beams");
      } else {
          ASKAPLOG_INFO_STR(logger, "No phase rotation will be done between iterations");
      }
  
      // load sky model, populate itsPerfectModel
      readModels();
      if (itsComms.isParallel()) {
          // setup work units in the parallel case, make beams the first (fastest to change) parameter to achieve
          // greater benefits if multiple measurement sets are present (more likely to be scheduled for different ranks)
          itsWorkUnitIterator.init(casa::IPosition(2, nBeam(), nChan()), itsComms.nProcs() - 1, itsComms.rank() - 1);
      } 
  } 
  if (!itsComms.isParallel()) {
      // setup work units in the serial case - all work to be done here
      itsWorkUnitIterator.init(casa::IPosition(2, nBeam(), nChan()));
  }
  ASKAPCHECK((measurementSets().size() == 1) || (measurementSets().size() == nBeam()), 
       "Number of measurement sets given in the parset ("<<measurementSets().size()<<
       ") should be either 1 or equal the number of beams ("<<nBeam()<<")");      
}          

/// @brief method which does the main job
/// @details it iterates over all channels/beams and writes the result.
/// In the parallel mode each worker iterates over their own portion of work and
/// then sends the result to master for writing.
void BPCalibratorParallel::run()
{
  if (itsComms.isWorker()) {
      ASKAPDEBUGASSERT(itsModel);
      ASKAPDEBUGASSERT(itsEquation);
      const int nCycles = parset().getInt32("ncycles", 1);
      ASKAPCHECK(nCycles >= 0, " Number of calibration iterations should be a non-negative number, you have " <<
                       nCycles);                                             
      for (itsWorkUnitIterator.origin(); itsWorkUnitIterator.hasMore(); itsWorkUnitIterator.next()) {
           // this will force creation of the new measurement equation for this beam/channel pair
           itsEquation.reset();
            
           const std::pair<casa::uInt, casa::uInt> indices = currentBeamAndChannel();
           
           ASKAPLOG_INFO_STR(logger, "Initialise bandpass (unknowns) for "<<nAnt()<<" antennas for beam="<<indices.first<<
                             " and channel="<<indices.second);
           itsModel->reset();                             
           for (casa::uInt ant = 0; ant<nAnt(); ++ant) {
                itsModel->add(accessors::CalParamNameHelper::paramName(ant, indices.first, casa::Stokes::XX), casa::Complex(1.,0.));
                itsModel->add(accessors::CalParamNameHelper::paramName(ant, indices.first, casa::Stokes::YY), casa::Complex(1.,0.));                
           }           
           
           for (int cycle = 0; cycle < nCycles; ++cycle) {
                ASKAPLOG_INFO_STR(logger, "*** Starting calibration iteration " << cycle + 1 << " for beam="<<
                              indices.first<<" and channel="<<indices.second<<" ***");                    
                // iterator is used to access the current work unit inside calcNE
                calcNE();
                solveNE();
           }
           if (itsComms.isParallel()) {
               // send to the master comes here
           } else {
               // serial operation, write comes here
               writeModel();
           }
      }     
  }
  if (itsComms.isMaster() && itsComms.isParallel()) {
      const casa::uInt numberOfWorkUnits = nBeam() * nChan();
      for (casa::uInt chunk = 0; chunk < numberOfWorkUnits; ++chunk) {
           // asynchronous receive from workers comes here
           writeModel();
      } 
  }
} 

/// @brief extract current beam/channel pair from the iterator
/// @details This method encapsulates interpretation of the output of itsWorkUnitIterator.cursor()
/// @return pair of beam (first) and channel (second) indices
std::pair<casa::uInt, casa::uInt> BPCalibratorParallel::currentBeamAndChannel() const
{
  const casa::IPosition cursor = itsWorkUnitIterator.cursor();
  ASKAPDEBUGASSERT(cursor.nelements() == 2);
  ASKAPDEBUGASSERT((cursor[0] >= 0) && (cursor[1] >= 0));
  const std::pair<casa::uInt,casa::uInt> result(static_cast<casa::uInt>(cursor[0]), static_cast<casa::uInt>(cursor[1]));
  ASKAPDEBUGASSERT(result.first < nBeam());
  ASKAPDEBUGASSERT(result.second < nChan());
  return result;
}


/// @brief Calculate the normal equations (runs in workers)
/// @details Model, either image-based or component-based, is used in conjunction with 
/// CalibrationME to calculate the generic normal equations. 
void BPCalibratorParallel::calcNE()
{
  ASKAPDEBUGASSERT(itsComms.isWorker());
  
  // create a new instance of the normal equations class
  boost::shared_ptr<scimath::GenericNormalEquations> gne(new scimath::GenericNormalEquations);
  itsNe = gne;
        
  ASKAPDEBUGASSERT(itsNe);
      
  // obtain details on the current iteration, i.e. beam and channel
  ASKAPDEBUGASSERT(itsWorkUnitIterator.hasMore());
  
  const std::pair<casa::uInt, casa::uInt> indices = currentBeamAndChannel();
  
  ASKAPDEBUGASSERT((measurementSets().size() == 1) || (indices.first < measurementSets().size()));
              
  const std::string ms = (measurementSets().size() == 1 ? measurementSets()[0] : measurementSets()[indices.first]);
        
  // actual computation   
  calcOne(ms, indices.first, indices.second);  
}

/// @brief Solve the normal equations (runs in workers)
/// @details Parameters of the calibration problem are solved for here
void BPCalibratorParallel::solveNE()
{
  if (itsComms.isWorker()) {        
      ASKAPLOG_INFO_STR(logger, "Solving normal equations");
      casa::Timer timer;
      timer.mark();
      scimath::Quality q;
      ASKAPDEBUGASSERT(itsSolver);
      itsSolver->init();
      itsSolver->addNormalEquations(*itsNe);
      itsSolver->setAlgorithm("SVD");     
      itsSolver->solveNormalEquations(*itsModel,q);
      ASKAPLOG_INFO_STR(logger, "Solved normal equations in "<< timer.real() << " seconds ");
      ASKAPLOG_INFO_STR(logger, "Solution quality: "<<q);
      if (itsRefGain != "") {
          ASKAPLOG_INFO_STR(logger, "Rotating phases to have that of "<<itsRefGain<<" equal to 0");
          rotatePhases();
      }
  }
}

/// @brief Write the results (runs in master)
/// @details The solution (calibration parameters) is reported via solution accessor
void BPCalibratorParallel::writeModel(const std::string &)
{
  ASKAPDEBUGASSERT(itsComms.isMaster());
  
  const std::pair<casa::uInt, casa::uInt> indices = currentBeamAndChannel();
  
  ASKAPLOG_INFO_STR(logger, "Writing results of the calibration for beam="<<indices.first<<" channel="<<indices.second);
  
  ASKAPCHECK(itsSolutionSource, "Solution source has to be defined by this stage");

  if (!itsSolutionIDValid) {
      // obtain solution ID only once, the results can come in random order and the
      // accessor is responsible for aggregating all of them together. This is done based on this ID.
      itsSolutionID = itsSolutionSource->newSolutionID(solutionTime());
      itsSolutionIDValid = true;
  }    
  
  boost::shared_ptr<accessors::ICalSolutionAccessor> solAcc = itsSolutionSource->rwSolution(itsSolutionID);
  ASKAPASSERT(solAcc);
        
  ASKAPDEBUGASSERT(itsModel); 
  std::vector<std::string> parlist = itsModel->freeNames();
  for (std::vector<std::string>::const_iterator it = parlist.begin(); it != parlist.end(); ++it) {
       const casa::Complex val = itsModel->complexValue(*it);           
       const std::pair<accessors::JonesIndex, casa::Stokes::StokesTypes> paramType = 
             accessors::CalParamNameHelper::parseParam(*it);
       // beam is also coded in the parameters, although we don't need it because the data are partitioned
       // just cross-check it  
       ASKAPDEBUGASSERT(static_cast<casa::uInt>(paramType.first.beam()) == indices.first);             
       solAcc->setBandpassElement(paramType.first, paramType.second, indices.second, val);
  }
}

/// @brief create measurement equation
/// @details This method initialises itsEquation with shared pointer to a proper type.
/// It uses internal flags to create a correct type (i.e. polarisation calibration or
/// just antenna-based gains). Parameters are passed directly to the constructor of 
/// CalibrationME template.
/// @param[in] dsi data shared iterator 
/// @param[in] perfectME uncorrupted measurement equation
void BPCalibratorParallel::createCalibrationME(const accessors::IDataSharedIter &dsi, 
                const boost::shared_ptr<IMeasurementEquation const> &perfectME)
{
   ASKAPDEBUGASSERT(itsModel);
   ASKAPDEBUGASSERT(perfectME);
   
   // it is handy to have a shared pointer to the base type because it is
   // not templated
   boost::shared_ptr<PreAvgCalMEBase> preAvgME;
   // solve as normal gains (rather than bandpass) because only one channel is supposed to be selected
   // this also opens a possibility to use several (e.g. 54 = coarse resolution) channels to get one gain
   // solution which is then replicated to all channels involved. We can also add frequency-dependent leakage, if
   // tests show it is required (currently it is not in the calibration model)
   preAvgME.reset(new CalibrationME<NoXPolGain, PreAvgCalMEBase>());           
   ASKAPDEBUGASSERT(preAvgME);
   
   preAvgME->accumulate(dsi,perfectME);
   itsEquation = preAvgME;
           
   // this is just because we bypass setting the model for the first major cycle
   // in the case without pre-averaging
   itsEquation->setParameters(*itsModel);
}                
  
/// @brief helper method to rotate all phases
/// @details This method rotates the phases of all gains in itsModel
/// to have the phase of itsRefGain exactly 0. This operation does
/// not seem to be necessary for SVD solvers, however it simplifies
/// "human eye" analysis of the results (otherwise the phase degeneracy
/// would make the solution different from the simulated gains).
/// @note The method throws exception if itsRefGain is not among
/// the parameters of itsModel
void BPCalibratorParallel::rotatePhases()
{
  // the intention is to rotate phases in worker (for this class)
  ASKAPDEBUGASSERT(itsComms.isWorker());
  ASKAPDEBUGASSERT(itsModel);
  
  ASKAPCHECK(itsModel->has(itsRefGain), "phase rotation to `"<<itsRefGain<<
             "` is impossible because this parameter is not present in the model");
  casa::Complex  refPhaseTerm = casa::polar(1.f,-arg(itsModel->complexValue(itsRefGain)));
                       
  std::vector<std::string> names(itsModel->freeNames());
  for (std::vector<std::string>::const_iterator it=names.begin(); it!=names.end();++it)  {
       const std::string parname = *it;
       if (parname.find("gain") != std::string::npos) {
           itsModel->update(parname, itsModel->complexValue(parname) * refPhaseTerm);
       } 
  }
}
      
/// @brief helper method to extract solution time from NE.
/// @details To be able to time tag the calibration solutions we add
/// start and stop times extracted from the dataset as metadata to normal
/// equations. It allows us to send these times to the master, which
/// ultimately writes the calibration solution. Otherwise, these times 
/// could only be obtained in workers who deal with the actual data.
/// @return solution time (seconds since 0 MJD)
/// @note if no start/stop time metadata are present in the normal equations
/// this method returns 0.
double BPCalibratorParallel::solutionTime() const
{
  // use the earliest time corresponding to the data used to make this calibration solution
  // to tag the solution. A request for any latest time than this would automatically 
  // extract this solution as most recent.
  ASKAPASSERT(itsNe);
  
  boost::shared_ptr<scimath::GenericNormalEquations> gne = boost::dynamic_pointer_cast<scimath::GenericNormalEquations>(itsNe);
  if (gne) {
      const scimath::Params& metadata = gne->metadata();
      if (metadata.has("min_time")) {
          return metadata.scalarValue("min_time");
      }
  }
  return 0.;
}

/// Calculate normal equations for one data set, channel and beam
/// @param[in] ms Name of data set
/// @param[in] chan channel to work with
/// @param[in] beam beam to work with
void BPCalibratorParallel::calcOne(const std::string& ms, const casa::uInt chan, const casa::uInt beam)
{
  casa::Timer timer;
  timer.mark();
  ASKAPLOG_INFO_STR(logger, "Calculating normal equations for " << ms <<" channel "<<chan<<" beam "<<beam);
  // First time around we need to generate the equation 
  if (!itsEquation) {
      ASKAPLOG_INFO_STR(logger, "Creating measurement equation" );
      accessors::TableDataSource ds(ms, accessors::TableDataSource::DEFAULT, dataColumn());
      ds.configureUVWMachineCache(uvwMachineCacheSize(),uvwMachineCacheTolerance());      
      accessors::IDataSelectorPtr sel=ds.createSelector();
      sel << parset();
      sel->chooseChannels(1,chan);
      sel->chooseFeed(beam);
      accessors::IDataConverterPtr conv=ds.createConverter();
      conv->setFrequencyFrame(getFreqRefFrame(), "Hz");
      conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
      // ensure that time is counted in seconds since 0 MJD
      conv->setEpochFrame();
      accessors::IDataSharedIter it=ds.createIterator(sel, conv);
      
      ASKAPCHECK(itsModel, "Initial assumption of parameters is not defined");
      
      if (!itsPerfectME) {
          ASKAPLOG_INFO_STR(logger, "Constructing measurement equation corresponding to the uncorrupted model");
          ASKAPCHECK(itsPerfectModel, "Uncorrupted model not defined");
          if (SynthesisParamsHelper::hasImage(itsPerfectModel)) {
              ASKAPCHECK(!SynthesisParamsHelper::hasComponent(itsPerfectModel),
                         "Image + component case has not yet been implemented");
              // have to create an image-specific equation        
              boost::shared_ptr<ImagingEquationAdapter> ieAdapter(new ImagingEquationAdapter);
              ASKAPCHECK(gridder(), "Gridder not defined");
              ieAdapter->assign<ImageFFTEquation>(*itsPerfectModel, gridder());
              itsPerfectME = ieAdapter;
          } else {
              // model is a number of components, don't need an adapter here
         
              // it doesn't matter which iterator is passed below. It is not used
              boost::shared_ptr<ComponentEquation> 
                  compEq(new ComponentEquation(*itsPerfectModel,it));
              itsPerfectME = compEq;
          }
      }
      // now we could've used class data members directly instead of passing them to createCalibrationME
      createCalibrationME(it,itsPerfectME);         
      ASKAPCHECK(itsEquation, "Equation is not defined");
  } else {
      ASKAPLOG_INFO_STR(logger, "Reusing measurement equation" );
      // we need to update the model held by measurement equation 
      // because it has been cloned at construction
      ASKAPCHECK(itsEquation, "Equation is not defined");
      ASKAPCHECK(itsModel, "Model is not defined");
      itsEquation->setParameters(*itsModel);
  }
  ASKAPCHECK(itsNe, "NormalEquations are not defined");
  itsEquation->calcEquations(*itsNe);
  ASKAPLOG_INFO_STR(logger, "Calculated normal equations for "<< ms << " channel "<<chan<<" beam" <<beam<<" in "<< timer.real()
                     << " seconds ");
}


} // namespace synthesis

} // namespace askap

