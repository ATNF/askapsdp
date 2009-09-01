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

// logging stuff
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".parallel");

// own includes
#include <parallel/CalibratorParallel.h>
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
#include <measurementequation/ImagingEquationAdapter.h>
#include <gridding/VisGridderFactory.h>

#include <Common/ParameterSet.h>

// casa includes
#include <casa/aips.h>
#include <casa/OS/Timer.h>


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace askap;
using namespace askap::scimath;
using namespace askap::synthesis;
using namespace askap::mwbase;

/// @brief Constructor from ParameterSet
/// @details The parset is used to construct the internal state. We could
/// also support construction from a python dictionary (for example).
/// The command line inputs are needed solely for MPI - currently no
/// application specific information is passed on the command line.
/// @param[in] argc Number of command line inputs
/// @param[in] argv Command line inputs
/// @param[in] parset ParameterSet for inputs
CalibratorParallel::CalibratorParallel(int argc, const char** argv,
        const LOFAR::ParameterSet& parset) :
      MEParallel(argc, argv), itsParset(parset), 
      itsPerfectModel(new scimath::Params())
{
  if (isMaster()) {
      // set up image handler
      SynthesisParamsHelper::setUpImageHandler(itsParset);
      
      // load sky model, propulate itsPerfectModel
      readModels();
      // itsModel has gain parameters for calibration, populate them with
      // an initial guess
      ASKAPDEBUGASSERT(itsModel); // should be initialized in SynParallel
      
      // initial assumption of the parameters
      const casa::uInt nAnt = 45; // hard coded at this stage 
      for (casa::uInt ant = 0; ant<nAnt; ++ant) {
           itsModel->add("gain.g11."+utility::toString(ant),casa::Complex(1.,0.));
           itsModel->add("gain.g22."+utility::toString(ant),casa::Complex(1.,0.));
           //itsModel->fix("gain.g11."+utility::toString(ant));
      }
      
      
      /// Create the solver  
      itsSolver.reset(new LinearSolver(*itsModel));
      ASKAPCHECK(itsSolver, "Solver not defined correctly");
      itsRefGain = itsParset.getString("refgain","");
  }
  if (isWorker()) {
      /// Get the list of measurement sets and the column to use.
      itsColName=itsParset.getString("datacolumn", "DATA");
      itsMs=itsParset.getStringVector("dataset");
      ASKAPCHECK(itsMs.size()>0, "Need dataset specification");
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
          ASKAPCHECK(int(itsMs.size()) == (itsNNode-1),
              "When running in parallel, need one data set per node");
      }

      /// Create the gridder using a factory acting on a
      /// parameterset
      itsGridder=VisGridderFactory::make(itsParset);
      ASKAPCHECK(itsGridder, "Gridder not defined correctly");
  }
}

/// @brief read the model from parset file and populate itsPerfectModel
/// @details This method is common between several classes and probably
/// should be pushed up in the class hierarchy
void CalibratorParallel::readModels()
{
  LOFAR::ParameterSet parset(itsParset);
  if(itsParset.isDefined("sources.definition")) {
    parset=LOFAR::ParameterSet(substitute(itsParset.getString("sources.definition")));
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
      TableDataSource ds(ms, TableDataSource::DEFAULT, itsColName);
      IDataSelectorPtr sel=ds.createSelector();
      sel << itsParset;
      IDataConverterPtr conv=ds.createConverter();
      conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
          "Hz");
      conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
      IDataSharedIter it=ds.createIterator(sel, conv);
      ASKAPCHECK(itsPerfectModel, "Uncorrupted model not defined");
      ASKAPCHECK(itsModel, "Initial assumption of parameters is not defined");
      ASKAPCHECK(itsGridder, "Gridder not defined");
      if (SynthesisParamsHelper::hasImage(itsPerfectModel)) {
         ASKAPCHECK(!SynthesisParamsHelper::hasComponent(itsPerfectModel),
                 "Image + component case has not yet been implemented");
         // have to create an image-specific equation        
         boost::shared_ptr<ImagingEquationAdapter> ieAdapter(new ImagingEquationAdapter);
         ieAdapter->assign<ImageFFTEquation>(*itsPerfectModel, itsGridder);
         itsEquation.reset(new CalibrationME<NoXPolGain>(*itsModel,it,ieAdapter));
      } else {
         // model is a number of components, don't need an adapter here
         
         // it doesn't matter which iterator is passed below. It is not used
         boost::shared_ptr<ComponentEquation> 
                  compEq(new ComponentEquation(*itsPerfectModel,it));
         itsEquation.reset(new CalibrationME<NoXPolGain>(*itsModel,it,compEq));         
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

/// Calculate the normal equations for a given measurement set
void CalibratorParallel::calcNE()
{
  /// Now we need to recreate the normal equations
  itsNe.reset(new GenericNormalEquations);

  if (isWorker()) {
        
      ASKAPDEBUGASSERT(itsNe);

      if (isParallel()) {
          calcOne(itsMs[itsRank-1]);
          sendNE();
      } else {
          ASKAPCHECK(itsSolver, "Solver not defined correctly");
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
        
      ASKAPLOG_INFO_STR(logger, "Solving normal equations");
      casa::Timer timer;
      timer.mark();
      Quality q;
      ASKAPDEBUGASSERT(itsSolver);
      itsSolver->setAlgorithm("SVD");     
      itsSolver->solveNormalEquations(q);
      ASKAPLOG_INFO_STR(logger, "Solved normal equations in "<< timer.real() << " seconds ");
      *itsModel=itsSolver->parameters();
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
  ASKAPDEBUGASSERT(isMaster());
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
  if (isMaster()) {
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

