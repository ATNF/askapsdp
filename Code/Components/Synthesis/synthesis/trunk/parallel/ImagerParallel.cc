///
/// @file 
///
/// Performs synthesis imaging from a data source, using any of a number of
/// image solvers. Can run in serial or parallel (MPI) mode.
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>

#include <parallel/ImagerParallel.h>

#include <askap_synthesis.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>
// need it just for null deleter
#include <askap/AskapUtil.h>

#include <askapparallel/AskapParallel.h>
#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/MEParsetInterface.h>
#include <measurementequation/CalibrationIterator.h>
#include <measurementequation/VoidMeasurementEquation.h>
#include <measurementequation/CalibrationME.h>
#include <measurementequation/NoXPolGain.h>
#include <measurementequation/ImageParamsHelper.h>
#include <fitting/Params.h>

#include <measurementequation/ImageSolverFactory.h>
#include <gridding/VisGridderFactory.h>

#include <casa/aips.h>
#include <casa/OS/Timer.h>

#include <Common/ParameterSet.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>

using namespace askap;
using namespace askap::scimath;
using namespace askap::synthesis;
using namespace askap::mwbase;

namespace askap
{
  namespace synthesis
  {

    ImagerParallel::ImagerParallel(askap::mwbase::AskapParallel& comms,
        const LOFAR::ParameterSet& parset) :
      MEParallel(comms), itsParset(parset),
      itsUVWMachineCacheSize(1), itsUVWMachineCacheTolerance(1e-6)
    {
      if (itsComms.isMaster())
      {
        // set up image handler
        SynthesisParamsHelper::setUpImageHandler(itsParset);
      
        itsRestore=itsParset.getBool("restore", false);
        
        bool reuseModel = itsParset.getBool("Images.reuse", false);
        
        itsUseMemoryBuffers = itsParset.getBool("memorybuffers", false);
        
        
        ASKAPCHECK(itsModel, "itsModel is supposed to be initialized at this stage");
        
        if (reuseModel) {
            ASKAPLOG_INFO_STR(logger, "Reusing model images stored on disk");
            SynthesisParamsHelper::loadImages(itsModel,itsParset.makeSubset("Images."));
        } else {
            ASKAPLOG_INFO_STR(logger, "Initializing the model images");
      
            /// Create the specified images from the definition in the
            /// parameter set. We can solve for any number of images
            /// at once (but you may/will run out of memory!)
            SynthesisParamsHelper::setUpImages(itsModel, 
                                      itsParset.makeSubset("Images."));
        }

        /// Create the solver from the parameterset definition and the existing
        /// definition of the parameters. 
        itsSolver=ImageSolverFactory::make(*itsModel, itsParset);
        ASKAPCHECK(itsSolver, "Solver not defined correctly");
      }
      if (itsComms.isWorker())
      {
        /// Get the list of measurement sets and the column to use.
        itsColName=itsParset.getString("datacolumn", "DATA");
        itsMs=itsParset.getStringVector("dataset");
        itsGainsFile = itsParset.getString("gainsfile","");
        const int cacheSize = itsParset.getInt32("nUVWMachines",1);
        ASKAPCHECK(cacheSize > 0 ,"Cache size is supposed to be a positive number, you have "<<cacheSize);
        itsUVWMachineCacheSize = size_t(cacheSize);
        itsUVWMachineCacheTolerance = 
            SynthesisParamsHelper::convertQuantity(itsParset.getString("uvwMachineDirTolerance", 
                                                   "1e-6rad"),"rad");
         
        ASKAPLOG_INFO_STR(logger, "UVWMachine cache will store "<<itsUVWMachineCacheSize<<" machines");
        ASKAPLOG_INFO_STR(logger, "Tolerance on the directions is "<<itsUVWMachineCacheTolerance/casa::C::pi*180.*3600.<<" arcsec");
        
        ASKAPCHECK(itsMs.size()>0, "Need dataset specification");
        const int nNodes = itsComms.nNodes();
        if (itsMs.size()==1)
        {
          string tmpl=itsMs[0];
          if (nNodes>2)
          {
            itsMs.resize(nNodes-1);
          }
          for (int i=0; i<nNodes-1; i++)
          {
            itsMs[i]=substitute(tmpl);
          }
        }
        if (nNodes>1)
        {
          ASKAPCHECK(int(itsMs.size()) == (nNodes-1),
              "When running in parallel, need one data set per node");
        }

        /// Create the gridder using a factory acting on a
        /// parameterset
        itsGridder=VisGridderFactory::make(itsParset);
        ASKAPCHECK(itsGridder, "Gridder not defined correctly");
      }
    }

    void ImagerParallel::calcOne(const string& ms, bool discard)
    {
      casa::Timer timer;
      timer.mark();
      ASKAPLOG_INFO_STR(logger, "Calculating normal equations for " << ms );
      // First time around we need to generate the equation 
      if ((!itsEquation)||discard)
      {
        ASKAPLOG_INFO_STR(logger, "Creating measurement equation" );

        // just to print the current mode to the log
        if (itsUseMemoryBuffers) {
            ASKAPLOG_INFO_STR(logger, "Scratch data will be held in memory" );
        } else {
            ASKAPLOG_INFO_STR(logger, "Scratch data will be written to the subtable of the original dataset" );
        }
        
        TableDataSource ds(ms, (itsUseMemoryBuffers ? TableDataSource::MEMORY_BUFFERS : TableDataSource::DEFAULT), 
                           itsColName);
        ds.configureUVWMachineCache(itsUVWMachineCacheSize,itsUVWMachineCacheTolerance);                   
        IDataSelectorPtr sel=ds.createSelector();
        sel << itsParset;
        IDataConverterPtr conv=ds.createConverter();
        conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
            "Hz");
        conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
        IDataSharedIter it=ds.createIterator(sel, conv);
        ASKAPCHECK(itsModel, "Model not defined");
        ASKAPCHECK(itsGridder, "Gridder not defined");
        if (!itsGainsFile.size()) {
            ASKAPLOG_INFO_STR(logger, "No calibration is applied" );
            itsEquation = askap::scimath::Equation::ShPtr(new ImageFFTEquation (*itsModel, it, itsGridder));
        } else {
            ASKAPLOG_INFO_STR(logger, "Calibration will be performed using gains from '"<<itsGainsFile<<"'");
            
            scimath::Params gainModel; 
            gainModel << LOFAR::ParameterSet(itsGainsFile);
	        /*
	        // temporary "matrix inversion". we need to do it properly in the
	        // CalibrationME class. The code below won't work for cross-pols
	        std::vector<std::string> names = gainModel.names();
	        for (std::vector<std::string>::const_iterator nameIt = names.begin();
	             nameIt != names.end(); ++nameIt) {
	                casa::Complex gain = gainModel.complexValue(*nameIt);
	                if (abs(gain)<1e-3) {
	                    ASKAPLOG_INFO_STR(logger, "Very small gain has been encountered "<<*nameIt
	                           <<"="<<gain);
	                    continue;       
	                } else {
	                  gainModel.update(*nameIt, casa::Complex(1.,0.)/gain);
	                }
	             }
	        //
	        */
	        if (!itsVoidME) {
	            itsVoidME.reset(new VoidMeasurementEquation);
	        }
	        // in the following statement it doesn't matter which iterator is passed
	        // to the class as long as it is valid (it is not used at all).
	        boost::shared_ptr<IMeasurementEquation> 
	               calME(new CalibrationME<NoXPolGain>(gainModel,it,itsVoidME));
            
            IDataSharedIter calIter(new CalibrationIterator(it,calME));
            itsEquation = askap::scimath::Equation::ShPtr(
                          new ImageFFTEquation (*itsModel, calIter, itsGridder));
        }
      }
      else {
        ASKAPLOG_INFO_STR(logger, "Reusing measurement equation and updating with latest model images" );
	itsEquation->setParameters(*itsModel);
      }
      ASKAPCHECK(itsEquation, "Equation not defined");
      ASKAPCHECK(itsNe, "NormalEquations not defined");
      itsEquation->calcEquations(*itsNe);
      ASKAPLOG_INFO_STR(logger, "Calculated normal equations for "<< ms << " in "<< timer.real()
                         << " seconds ");
    }

    /// Calculate the normal equations for a given measurement set
    void ImagerParallel::calcNE()
    {
      /// Now we need to recreate the normal equations
      itsNe=ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*itsModel));

      if (itsComms.isWorker())
      {
        ASKAPCHECK(itsGridder, "Gridder not defined");
        ASKAPCHECK(itsModel, "Model not defined");
        //				ASKAPCHECK(itsMs.size()>0, "Data sets not defined");

        ASKAPCHECK(itsNe, "NormalEquations not defined");

        if (itsComms.isParallel())
        {
          calcOne(itsMs[itsComms.rank()-1]);
          sendNE();
        }
        else
        {
          ASKAPCHECK(itsSolver, "Solver not defined correctly");
          itsSolver->init();
          itsSolver->setParameters(*itsModel);
          for (size_t iMs=0; iMs<itsMs.size(); iMs++)
          {
            calcOne(itsMs[iMs]);
            itsSolver->addNormalEquations(*itsNe);
          }
        }
      }
    }

    void ImagerParallel::solveNE()
    {
      if (itsComms.isMaster())
      {
        // Receive the normal equations
        if (itsComms.isParallel())
        {
          receiveNE();
        }
        ASKAPLOG_INFO_STR(logger, "Solving normal equations");
        casa::Timer timer;
        timer.mark();
        Quality q;
        itsSolver->solveNormalEquations(q);
        ASKAPLOG_INFO_STR(logger, "Solved normal equations in "<< timer.real() << " seconds "
                           );
        ASKAPDEBUGASSERT(itsModel);
        *itsModel=itsSolver->parameters();
        
        // we will probably send all of them out in the future, but for now
        // let's extract the largest residual
        const std::vector<std::string> peakParams = itsModel->completions("peak_residual.");
        
        double peak = peakParams.size() == 0 ? getPeakResidual() : -1.;
        for (std::vector<std::string>::const_iterator peakParIt = peakParams.begin();
             peakParIt != peakParams.end(); ++peakParIt) {
             const double tempval = std::abs(itsModel->scalarValue("peak_residual."+*peakParIt));
             ASKAPLOG_INFO_STR(logger, "Peak residual for "<< *peakParIt << " is "<<tempval);
             if (tempval > peak) {
                 peak = tempval;
             }
        }
        
        if (itsModel->has("peak_residual")) {
            itsModel->update("peak_residual",peak);
        } else {
            itsModel->add("peak_residual",peak);
        }
        itsModel->fix("peak_residual");
      }
    }
    
    /// @brief Helper method to zero all model images
    /// @details We need this for dirty solver only, as otherwise restored image 
    /// (which is crucial for faceting) will be wrong.
    void ImagerParallel::zeroAllModelImages() const 
    {
      ASKAPCHECK(itsModel, "Model should not be empty at this stage!");
      ASKAPLOG_INFO_STR(logger, "Dirty solver mode, setting all model images to 0."); 
      SynthesisParamsHelper::zeroAllModelImages(itsModel);
    }
    
    
    /// @brief a helper method to extract peak residual
    /// @details This object actually manipulates with the normal equations. We need
    /// to be able to stop iterations on the basis of maximum residual, which is a 
    /// data vector of the normal equations. This helper method is designed to extract
    /// peak residual. It is then added to a model as a parameter (the model is 
    /// shipped around).
    /// @return absolute value peak of the residuals corresponding to the current normal
    /// equations
    double ImagerParallel::getPeakResidual() const
    {
      ASKAPDEBUGASSERT(itsNe);
      // we need a specialized method of the imaging normal equations to get the peak
      // for all images. Multiple images can be represented by a single normal equations class.
      // We could also use the dataVector method of the interface (INormalEquations). However,
      // it is a bit cumbersome to iterate over all parameters. It is probably better to
      // leave this full case for a future as there is no immediate use case.
      boost::shared_ptr<ImagingNormalEquations> ine = 
                    boost::dynamic_pointer_cast<ImagingNormalEquations>(itsNe);
      // we could have returned some special value (e.g. negative), but throw exception for now              
      ASKAPCHECK(ine, "Current code to calculate peak residuals works for imaging-specific normal equations only");              
      double peak = -1.; 
      const std::map<string, casa::Vector<double> >& dataVector = ine->dataVector();
      const std::map<string, casa::Vector<double> >& diag = ine->normalMatrixDiagonal();
      for (std::map<string, casa::Vector<double> >::const_iterator ci = dataVector.begin();
           ci!=dataVector.end(); ++ci) {
           if (ci->first.find("image") == 0) {
               // this is an image
               ASKAPASSERT(ci->second.nelements() != 0);               
               std::map<std::string, casa::Vector<double> >::const_iterator diagIt = 
                            diag.find(ci->first);
               ASKAPDEBUGASSERT(diagIt != diag.end());
               const double maxDiag = casa::max(diagIt->second);
               // hard coded at this stage
               const double cutoff=1e-2*maxDiag;
               ASKAPDEBUGASSERT(diagIt->second.nelements() == ci->second.nelements());               
               for (casa::uInt elem = 0; elem<diagIt->second.nelements(); ++elem) {
                    const double thisDiagElement = std::abs(diagIt->second[elem]);
                    if (thisDiagElement > cutoff) {
                        const double tempPeak =  ci->second[elem]/thisDiagElement;
                        if (tempPeak > peak) {
                            peak = tempPeak;
                        }
                    }
               }
           }
      }
      return peak;
    }

    /// Write the results out
    /// @param[in] postfix this string is added to the end of each name
    /// (used to separate images at different iterations)
    void ImagerParallel::writeModel(const std::string &postfix)
    {
      if (itsComms.isMaster())
      {
        ASKAPLOG_INFO_STR(logger, "Writing out results as images");
        vector<string> resultimages=itsModel->names();
        for (vector<string>::const_iterator it=resultimages.begin(); it
            !=resultimages.end(); it++) {
            if ((it->find("image") == 0) || (it->find("psf") == 0) ||
                (it->find("weights") == 0) || (it->find("mask") == 0) ||
                (it->find("residual")==0)) {
                ASKAPLOG_INFO_STR(logger, "Saving " << *it << " with name " << *it+postfix );
                SynthesisParamsHelper::saveImageParameter(*itsModel, *it, *it+postfix);
            }
        }

        if (itsRestore && postfix == "")
        {
          ASKAPLOG_INFO_STR(logger, "Writing out restored images as CASA images");
          ASKAPDEBUGASSERT(itsModel);
          boost::shared_ptr<ImageRestoreSolver> ir = ImageRestoreSolver::createSolver(itsParset.makeSubset("restore."), 
                                                                *itsModel);
          ASKAPDEBUGASSERT(ir);
          ASKAPDEBUGASSERT(itsSolver);
          // configure restore solver the same way as normal imaging solver
          boost::shared_ptr<ImageSolver> template_solver = boost::dynamic_pointer_cast<ImageSolver>(itsSolver);
          ASKAPDEBUGASSERT(template_solver);
          ir->configureSolver(*template_solver);
          ir->copyNormalEquations(*template_solver);
          Quality q;
          ir->solveNormalEquations(q);
          *itsModel = ir->parameters();
          // merged image should be a fixed parameter without facet suffixes
          resultimages=itsModel->fixedNames();
          for (vector<string>::const_iterator ci=resultimages.begin(); ci!=resultimages.end(); ++ci) {
               const ImageParamsHelper iph(*ci);
               if (!iph.isFacet() && (ci->find("image") == 0)) {
                   ASKAPLOG_INFO_STR(logger, "Saving restored image " << *ci << " with name "
                               << *ci+string(".restored") );
                   SynthesisParamsHelper::saveImageParameter(*itsModel, *ci,*ci+string(".restored"));
               }
          }
        }
      }
    }

  }
}
