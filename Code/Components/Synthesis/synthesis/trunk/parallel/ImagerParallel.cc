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
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>
#include <askap_synthesis.h>

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
#include <fitting/Params.h>

// These three includes can be removed when ImageRestoreSolver goes out of here
#include <measurementequation/IImagePreconditioner.h>
#include <measurementequation/WienerPreconditioner.h>
#include <measurementequation/GaussianTaperPreconditioner.h>

#include <measurementequation/ImageSolverFactory.h>
#include <gridding/VisGridderFactory.h>

#include <casa/aips.h>
#include <casa/OS/Timer.h>

#include <APS/ParameterSet.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>

using namespace askap;
using namespace askap::scimath;
using namespace askap::synthesis;
using namespace LOFAR::ACC::APS;
using namespace askap::cp;

namespace askap
{
  namespace synthesis
  {

    ImagerParallel::ImagerParallel(int argc, const char** argv,
        const LOFAR::ACC::APS::ParameterSet& parset) :
      MEParallel(argc, argv), itsParset(parset)
    {

      if (isMaster())
      {
        itsRestore=itsParset.getBool("restore", false);

        if (itsRestore) {
            itsQbeam.resize(3);
            vector<string> beam = itsParset.getStringVector("restore.beam");
            ASKAPCHECK(beam.size() == 3, "Need three elements for beam");
            for (int i=0; i<3; ++i) {
                 casa::Quantity::read(itsQbeam(i), beam[i]);
            }
        }
        
        bool reuseModel = itsParset.getBool("Images.reuse", false);
        
        itsUseMemoryBuffers = itsParset.getBool("memorybuffers", false);
        
        
        ASKAPCHECK(itsModel, "itsModel is supposed to be initialized at this stage");
        
        if (reuseModel) {
            ASKAPLOG_INFO_STR(logger, "Reusing model images stored on disk");
            const vector<string> images=parset.getStringVector("Images.Names");
            for (vector<string>::const_iterator ci = images.begin(); ci != images.end(); ++ci) {
                 ASKAPLOG_INFO_STR(logger, "Reading model image "<<*ci);
                 SynthesisParamsHelper::getFromCasaImage(*itsModel,*ci,*ci);
            }            
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
      if (isWorker())
      {
        /// Get the list of measurement sets and the column to use.
        itsColName=itsParset.getString("datacolumn", "DATA");
        itsMs=itsParset.getStringVector("dataset");
        itsGainsFile = itsParset.getString("gainsfile","");
        ASKAPCHECK(itsMs.size()>0, "Need dataset specification");
        if (itsMs.size()==1)
        {
          string tmpl=itsMs[0];
          if (itsNNode>2)
          {
            itsMs.resize(itsNNode-1);
          }
          for (int i=0; i<itsNNode-1; i++)
          {
            itsMs[i]=substitute(tmpl);
          }
        }
        if (itsNNode>1)
        {
          ASKAPCHECK(int(itsMs.size()) == (itsNNode-1),
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
	        gainModel << ParameterSet(itsGainsFile);
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

      if (isWorker())
      {
        ASKAPCHECK(itsGridder, "Gridder not defined");
        ASKAPCHECK(itsModel, "Model not defined");
        //				ASKAPCHECK(itsMs.size()>0, "Data sets not defined");

        ASKAPCHECK(itsNe, "NormalEquations not defined");

        if (isParallel())
        {
          calcOne(itsMs[itsRank-1]);
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
      if (isMaster())
      {
        // Receive the normal equations
        if (isParallel())
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
      if (isMaster())
      {
        ASKAPLOG_INFO_STR(logger, "Writing out results as CASA images");
        vector<string> resultimages=itsModel->names();
        for (vector<string>::const_iterator it=resultimages.begin(); it
            !=resultimages.end(); it++) {
            if ((it->find("image") == 0) || (it->find("psf") == 0) ||
                (it->find("weights") == 0) || (it->find("mask") == 0)) {
                ASKAPLOG_INFO_STR(logger, "Saving " << *it << " with name " << *it+postfix );
                SynthesisParamsHelper::saveAsCasaImage(*itsModel, *it, *it+postfix);
            }
        }

        if (itsRestore && postfix == "")
        {
          ASKAPLOG_INFO_STR(logger, "Writing out restored images as CASA images");
          ImageRestoreSolver ir(*itsModel, itsQbeam);
          ir.setThreshold(itsSolver->threshold());
          ir.setVerbose(itsSolver->verbose());
	  /// @todo Fix copying of preconditioners
	  // Check for preconditioners. Same code as in ImageSolverFactory.
	  // Will be neater if the RestoreSolver is also created in the ImageSolverFactory.
	  const vector<string> preconditioners=itsParset.getStringVector("preconditioner.Names",std::vector<std::string>());
	  if(preconditioners.size()) {
	    for (vector<string>::const_iterator pc = preconditioners.begin(); pc != preconditioners.end(); ++pc) {
	      if( (*pc)=="Wiener" ) {
	        float noisepower = itsParset.getFloat("preconditioner.Wiener.noisepower",0.0);
		ir.addPreconditioner(IImagePreconditioner::ShPtr(new WienerPreconditioner(noisepower)));
	      }
	      if ( (*pc) == "GaussianTaper") {
	          // at this stage we have to define tapers in uv-cells, rather than in klambda
	          // because the physical cell size is unknown to solver factory. 
	          // Theoretically we could parse the parameters here and extract the cell size and
	          // shape, but it can be defined separately for each image. We need to find
	          // the way of dealing with this complication.
	          ASKAPCHECK(itsParset.isDefined("preconditioner.GaussianTaper"), 
	                "preconditioner.GaussianTaper showwing the taper size should be defined to use GaussianTaper");
	          const vector<double> taper = SynthesisParamsHelper::convertQuantity(
	                 itsParset.getStringVector("preconditioner.GaussianTaper"),"rad");
	          ASKAPCHECK((taper.size() == 3) || (taper.size() == 1), 
	                     "preconditioner.GaussianTaper can have either single element or "
	                     " a vector of 3 elements. You supplied a vector of "<<taper.size()<<" elements");     
	          ASKAPCHECK(itsParset.isDefined("Images.shape") && itsParset.isDefined("Images.cellsize"),
	                 "Imager.shape and Imager.cellsize should be defined to convert the taper fwhm specified in "
	                 "angular units in the image plane into uv cells");
	          const std::vector<double> cellsize = SynthesisParamsHelper::convertQuantity(
	                    itsParset.getStringVector("Images.cellsize"),"rad");
	          const std::vector<int> shape = itsParset.getInt32Vector("Images.shape");
	          ASKAPCHECK((cellsize.size() == 2) && (shape.size() == 2), 
	              "Images.cellsize and Images.shape parameters should have exactly two values");
	          // factors which appear in nominator are effectively half sizes in radians
	          const double xFactor = cellsize[0]*double(shape[0])/2.;
	          const double yFactor = cellsize[1]*double(shape[1])/2.;
	          
	          if (taper.size() == 3) {

	              ASKAPDEBUGASSERT((taper[0]!=0) && (taper[1]!=0));              
	              ir.addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(
	                     xFactor/taper[0],yFactor/taper[1],taper[2])));	                         
	          } else {
	              ASKAPDEBUGASSERT(taper[0]!=0);              	              
                  if (std::abs(xFactor-yFactor)<4e-15) {
                      // the image is square, can use the short cut
	                  ir.addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(xFactor/taper[0])));	                                
                  } else {
                      // the image is rectangular. Although the gaussian taper is symmetric in
                      // angular coordinates, it will be elongated along the vertical axis in 
                      // the uv-coordinates.
	                  ir.addPreconditioner(IImagePreconditioner::ShPtr(new GaussianTaperPreconditioner(xFactor/taper[0],
	                         yFactor/taper[0],0.)));	                                                      
                  } 
	          }          
	      }
	    }
	  }
	  else {
		  ir.addPreconditioner(IImagePreconditioner::ShPtr(new WienerPreconditioner()));
	  }

	  
          ir.copyNormalEquations(*itsSolver);
          Quality q;
          ir.solveNormalEquations(q);
          resultimages=ir.parameters().completions("image");
          for (vector<string>::iterator it=resultimages.begin(); it
              !=resultimages.end(); it++)
          {
            string imageName("image"+(*it)+postfix);
            ASKAPLOG_INFO_STR(logger, "Saving restored image " << imageName << " with name "
                               << imageName+string(".restored") );
            SynthesisParamsHelper::saveAsCasaImage(*itsModel, "image"+(*it),
                imageName+string(".restored"));
          }
        }
      }
    }

  }
}
