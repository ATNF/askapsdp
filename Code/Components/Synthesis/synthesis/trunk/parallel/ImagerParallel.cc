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
/// (c) 2007 ASKAP, All Rights Reserved.
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
        itsRestore=itsParset.getBool("restore", true);

        if (itsRestore) {
            itsQbeam.resize(3);
            vector<string> beam = itsParset.getStringVector("restore.beam");
            ASKAPCHECK(beam.size() == 3, "Need three elements for beam");
            for (int i=0; i<3; ++i) {
                 casa::Quantity::read(itsQbeam(i), beam[i]);
            }
        }
        
        bool reuseModel = itsParset.getBool("Images.reuse", false);

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
        TableDataSource ds(ms, TableDataSource::DEFAULT, itsColName);
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
        *itsModel=itsSolver->parameters();
      }
    }

    /// Write the results out
    void ImagerParallel::writeModel()
    {
      if (isMaster())
      {
        ASKAPLOG_INFO_STR(logger, "Writing out results as CASA images");
        vector<string> resultimages=itsModel->names();
        for (vector<string>::iterator it=resultimages.begin(); it
            !=resultimages.end(); it++)
        {
          ASKAPLOG_INFO_STR(logger, "Saving " << *it << " with name " << *it );
          SynthesisParamsHelper::saveAsCasaImage(*itsModel, *it, *it);
        }

        if (itsRestore)
        {
          ASKAPLOG_INFO_STR(logger, "Writing out restored images as CASA images");
          const float weinerparam=itsParset.getFloat("solver.Clean.robust",0.0);
          ImageRestoreSolver ir(*itsModel, itsQbeam, weinerparam);
          ir.setThreshold(itsSolver->threshold());
          ir.setVerbose(itsSolver->verbose());
          ir.copyNormalEquations(*itsSolver);
          Quality q;
          ir.solveNormalEquations(q);
          resultimages=ir.parameters().completions("image");
          for (vector<string>::iterator it=resultimages.begin(); it
              !=resultimages.end(); it++)
          {
            string imageName("image"+(*it));
            ASKAPLOG_INFO_STR(logger, "Saving restored image " << imageName << " with name "
                               << imageName+string(".restored") );
            SynthesisParamsHelper::saveAsCasaImage(*itsModel, imageName,
                imageName+string(".restored"));
          }
        }
      }
    }

  }
}
