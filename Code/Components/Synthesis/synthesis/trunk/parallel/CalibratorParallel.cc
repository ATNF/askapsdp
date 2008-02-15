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
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

// logging stuff
#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
CONRAD_LOGGER(logger, ".measurementequation");

// own includes
#include <parallel/CalibratorParallel.h>
#include <conrad/ConradError.h>
#include <conrad/ConradUtil.h>

#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>

#include <fitting/LinearSolver.h>
#include <fitting/GenericNormalEquations.h>

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/MEParsetInterface.h>
#include <measurementequation/CalibrationME.h>
#include <measurementequation/NoXPolGain.h>
#include <measurementequation/ImagingEquationAdapter.h>
#include <gridding/VisGridderFactory.h>

#include <APS/ParameterSet.h>

// casa includes
#include <casa/aips.h>
#include <casa/OS/Timer.h>


#include <iostream>
#include <fstream>
#include <sstream>

using namespace conrad;
using namespace conrad::scimath;
using namespace conrad::synthesis;
using namespace LOFAR::ACC::APS;
using namespace conrad::cp;

/// @brief Constructor from ParameterSet
/// @details The parset is used to construct the internal state. We could
/// also support construction from a python dictionary (for example).
/// The command line inputs are needed solely for MPI - currently no
/// application specific information is passed on the command line.
/// @param[in] argc Number of command line inputs
/// @param[in] argv Command line inputs
/// @param[in] parset ParameterSet for inputs
CalibratorParallel::CalibratorParallel(int argc, const char** argv,
        const LOFAR::ACC::APS::ParameterSet& parset) :
      MEParallel(argc, argv), itsParset(parset)
{
  if (isMaster()) {
      // load sky model, propulate itsPerfectModel
      readModels();
      // itsModel has gain parameters for calibration, populate them with
      // an initial guess
      CONRADDEBUGASSERT(itsModel); // should be initialized in SynParallel
      
      // initial assumption of the parameters
      const casa::uInt nAnt = 45; // hard coded at this stage 
      for (casa::uInt ant = 0; ant<nAnt; ++ant) {
           itsModel->add("gain.g11"+utility::toString(ant),casa::Complex(1.,0.));
           itsModel->add("gain.g22"+utility::toString(ant),casa::Complex(1.,0.));
      }
      
      
      /// Create the solver  
      itsSolver.reset(new LinearSolver(*itsModel));
      CONRADCHECK(itsSolver, "Solver not defined correctly");
  }
  if (isWorker()) {
      /// Get the list of measurement sets and the column to use.
      itsColName=itsParset.getString("datacolumn", "DATA");
      itsMs=itsParset.getStringVector("dataset");
      CONRADCHECK(itsMs.size()>0, "Need dataset specification");
      if (itsMs.size()==1) {
          string tmpl=itsMs[0];
          if (itsNNode>2) {
            itsMs.resize(itsNNode-1);
          }
          for (int i=0; i<itsNNode-1; i++) {
            itsMs[i]=substitute(tmpl);
          }
      }
      if (itsNNode>1) {
          CONRADCHECK(int(itsMs.size()) == (itsNNode-1),
              "When running in parallel, need one data set per node");
      }

      /// Create the gridder using a factory acting on a
      /// parameterset
      itsGridder=VisGridderFactory::make(itsParset);
      CONRADCHECK(itsGridder, "Gridder not defined correctly");
  }
}

/// @brief read the model from parset file and populate itsPerfectModel
/// @details This method is common between several classes and probably
/// should be pushed up in the class hierarchy
void CalibratorParallel::readModels()
{
  ParameterSet parset(itsParset);
  if(itsParset.isDefined("sources.definition")) {
	  parset=ParameterSet(substitute(itsParset.getString("sources.definition")));
  }
      
  const std::vector<std::string> sources = parset.getStringVector("sources.names");
  for (size_t i=0; i<sources.size(); ++i) {
	   std::ostringstream oos;
	   oos << "sources." << sources[i]<< ".model";
	   if (parset.isDefined(oos.str())) {
           string model=parset.getString(oos.str());
           CONRADLOG_INFO_STR(logger, "Adding image " << model << " as model for "<< sources[i] );
           std::ostringstream paramName;
           paramName << "image.i." << sources[i];
           SynthesisParamsHelper::getFromCasaImage(*itsModel, paramName.str(), model);
       }
  }
  CONRADLOG_INFO_STR(logger, "Successfully read models");
}

void CalibratorParallel::calcOne(const std::string& ms, bool discard)
{
  casa::Timer timer;
  timer.mark();
  CONRADLOG_INFO_STR(logger, "Calculating normal equations for " << ms );
  // First time around we need to generate the equation 
  if ((!itsEquation)|| (!itsPerfectME) || discard) {
      CONRADLOG_INFO_STR(logger, "Creating measurement equation" );
      TableDataSource ds(ms, TableDataSource::DEFAULT, itsColName);
      IDataSelectorPtr sel=ds.createSelector();
      sel << itsParset;
      IDataConverterPtr conv=ds.createConverter();
      conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
          "Hz");
      conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
      IDataSharedIter it=ds.createIterator(sel, conv);
      CONRADCHECK(itsPerfectModel, "Uncorrupted model not defined");
      CONRADCHECK(itsModel, "Initial assumption of parameters is not defined");
      CONRADCHECK(itsGridder, "Gridder not defined");
      boost::shared_ptr<ImagingEquationAdapter> ieAdapter(new ImagingEquationAdapter);
      ieAdapter->assign<ImageFFTEquation>(*itsPerfectModel, itsGridder);
      itsPerfectME = ieAdapter;
      itsEquation.reset(new CalibrationME<NoXPolGain>(*itsModel,it,*itsPerfectME));
  } else {
      CONRADLOG_INFO_STR(logger, "Reusing measurement equation" );
  }
  CONRADCHECK(itsEquation, "Equation not defined");
  CONRADCHECK(itsPerfectME, "PerfectME not defined");
  CONRADCHECK(itsNe, "NormalEquations not defined");
  itsEquation->calcEquations(*itsNe);
  CONRADLOG_INFO_STR(logger, "Calculated normal equations for "<< ms << " in "<< timer.real()
                     << " seconds ");
}

/// Calculate the normal equations for a given measurement set
void CalibratorParallel::calcNE()
{
  /// Now we need to recreate the normal equations
  itsNe.reset(new GenericNormalEquations);

  if (isWorker()) {
        
      CONRADDEBUGASSERT(itsNe);

      if (isParallel()) {
          calcOne(itsMs[itsRank-1]);
          sendNE();
      } else {
          CONRADCHECK(itsSolver, "Solver not defined correctly");
          itsSolver->init();
          itsSolver->setParameters(*itsModel);
          for (size_t iMs=0; iMs<itsMs.size(); ++iMs) {
            calcOne(itsMs[iMs]);
            itsSolver->addNormalEquations(*itsNe);
          }
      }
  }
}

void CalibratorParallel::solveNE()
{
  if (isMaster()) {
      // Receive the normal equations
      if (isParallel()) {
          receiveNE();
      }
        
      CONRADLOG_INFO_STR(logger, "Solving normal equations");
      casa::Timer timer;
      timer.mark();
      Quality q;
      CONRADDEBUGASSERT(itsSolver);
      itsSolver->solveNormalEquations(q);
      CONRADLOG_INFO_STR(logger, "Solved normal equations in "<< timer.real() << " seconds ");
      *itsModel=itsSolver->parameters();
  }
}

/// Write the results out
void CalibratorParallel::writeModel()
{
  if (isMaster()) {
      CONRADLOG_INFO_STR(logger, "Writing out results into a parset file");
      CONRADDEBUGASSERT(itsModel);
      std::vector<std::string> parlist = itsModel->names();
      std::ofstream os("result.dat"); // for now just hard code it
      for (std::vector<std::string>::const_iterator it = parlist.begin(); 
           it != parlist.end(); ++it) {
           const casa::Complex gain = itsModel->complexValue(*it);
           os<<*it<<" = ["<<real(gain)<<","<<imag(gain)<<"]"<<std::endl;
      }
  }
}

