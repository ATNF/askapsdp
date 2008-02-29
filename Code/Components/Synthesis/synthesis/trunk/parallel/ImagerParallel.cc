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
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>

#include <parallel/ImagerParallel.h>

#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
CONRAD_LOGGER(logger, ".measurementequation");

#include <conrad/ConradError.h>
#include <conrad_synthesis.h>

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

using namespace conrad;
using namespace conrad::scimath;
using namespace conrad::synthesis;
using namespace LOFAR::ACC::APS;
using namespace conrad::cp;

namespace conrad
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

	if(itsRestore) {
	  itsQbeam.resize(3);
	  vector<string> beam=itsParset.getStringVector("restore.beam");
	  CONRADCHECK(beam.size()==3, "Need three elements for beam");
	  for (int i=0; i<3; i++)
	    {
	      casa::Quantity::read(itsQbeam(i), beam[i]);
	    }
	}

        /// Create the specified images from the definition in the
        /// parameter set. We can solve for any number of images
        /// at once (but you may/will run out of memory!)
        SynthesisParamsHelper::setUpImages(itsModel, 
                                           itsParset.makeSubset("Images."));
        CONRADCHECK(itsModel, "Model not defined correctly");

        /// Create the solver from the parameterset definition and the existing
        /// definition of the parameters. 
        itsSolver=ImageSolverFactory::make(*itsModel, itsParset);
        CONRADCHECK(itsSolver, "Solver not defined correctly");
      }
      if (isWorker())
      {
        /// Get the list of measurement sets and the column to use.
        itsColName=itsParset.getString("datacolumn", "DATA");
        itsMs=itsParset.getStringVector("dataset");
        itsGainsFile = itsParset.getString("gainsfile","");
        CONRADCHECK(itsMs.size()>0, "Need dataset specification");
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
          CONRADCHECK(int(itsMs.size()) == (itsNNode-1),
              "When running in parallel, need one data set per node");
        }

        /// Create the gridder using a factory acting on a
        /// parameterset
        itsGridder=VisGridderFactory::make(itsParset);
        CONRADCHECK(itsGridder, "Gridder not defined correctly");
      }
    }

    void ImagerParallel::calcOne(const string& ms, bool discard)
    {
      casa::Timer timer;
      timer.mark();
      CONRADLOG_INFO_STR(logger, "Calculating normal equations for " << ms );
      // First time around we need to generate the equation 
      if ((!itsEquation)||discard)
      {
        CONRADLOG_INFO_STR(logger, "Creating measurement equation" );
        TableDataSource ds(ms, TableDataSource::DEFAULT, itsColName);
        IDataSelectorPtr sel=ds.createSelector();
        sel << itsParset;
        IDataConverterPtr conv=ds.createConverter();
        conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
            "Hz");
        conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
        IDataSharedIter it=ds.createIterator(sel, conv);
        CONRADCHECK(itsModel, "Model not defined");
        CONRADCHECK(itsGridder, "Gridder not defined");
        if (!itsGainsFile.size()) {
            CONRADLOG_INFO_STR(logger, "No calibration is applied" );
            itsEquation = conrad::scimath::Equation::ShPtr(new ImageFFTEquation (*itsModel, it, itsGridder));
        } else {
            CONRADLOG_INFO_STR(logger, "Calibration will be performed using gains from '"<<itsGainsFile<<"'");
            
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
	                    CONRADLOG_INFO_STR(logger, "Very small gain has been encountered "<<*nameIt
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
	               calME(new CalibrationME<NoXPolGain>(gainModel,it,*itsVoidME));
            
            IDataSharedIter calIter(new CalibrationIterator(it,calME));
            itsEquation = conrad::scimath::Equation::ShPtr(
                          new ImageFFTEquation (*itsModel, calIter, itsGridder));
        }
      }
      else {
        CONRADLOG_INFO_STR(logger, "Reusing measurement equation" );
      }
      CONRADCHECK(itsEquation, "Equation not defined");
      CONRADCHECK(itsNe, "NormalEquations not defined");
      itsEquation->calcEquations(*itsNe);
      CONRADLOG_INFO_STR(logger, "Calculated normal equations for "<< ms << " in "<< timer.real()
                         << " seconds ");
    }

    /// Calculate the normal equations for a given measurement set
    void ImagerParallel::calcNE()
    {
      /// Now we need to recreate the normal equations
      itsNe=ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*itsModel));

      if (isWorker())
      {
        CONRADCHECK(itsGridder, "Gridder not defined");
        CONRADCHECK(itsModel, "Model not defined");
        //				CONRADCHECK(itsMs.size()>0, "Data sets not defined");

        CONRADCHECK(itsNe, "NormalEquations not defined");

        if (isParallel())
        {
          calcOne(itsMs[itsRank-1]);
          sendNE();
        }
        else
        {
          CONRADCHECK(itsSolver, "Solver not defined correctly");
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
        CONRADLOG_INFO_STR(logger, "Solving normal equations");
        casa::Timer timer;
        timer.mark();
        Quality q;
        itsSolver->solveNormalEquations(q);
        CONRADLOG_INFO_STR(logger, "Solved normal equations in "<< timer.real() << " seconds "
                           );
        *itsModel=itsSolver->parameters();
      }
    }

    /// Write the results out
    void ImagerParallel::writeModel()
    {
      if (isMaster())
      {
        CONRADLOG_INFO_STR(logger, "Writing out results as CASA images");
        vector<string> resultimages=itsModel->names();
        for (vector<string>::iterator it=resultimages.begin(); it
            !=resultimages.end(); it++)
        {
          CONRADLOG_INFO_STR(logger, "Saving " << *it << " with name " << *it );
          SynthesisParamsHelper::saveAsCasaImage(*itsModel, *it, *it);
        }

        if (itsRestore)
        {
          CONRADLOG_INFO_STR(logger, "Writing out restored images as CASA images");
          ImageRestoreSolver ir(*itsModel, itsQbeam);
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
            CONRADLOG_INFO_STR(logger, "Saving restored image " << imageName << " with name "
                               << imageName+string(".restored") );
            SynthesisParamsHelper::saveAsCasaImage(*itsModel, imageName,
                imageName+string(".restored"));
          }
        }
      }
    }

  }
}
