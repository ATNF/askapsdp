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

#include <conrad/ConradError.h>

#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/MEParsetInterface.h>

#include <measurementequation/ImageSolverFactory.h>
#include <gridding/VisGridderFactory.h>

#include <casa/aips.h>
#include <casa/OS/Timer.h>

#include <APS/ParameterSet.h>

#include <stdexcept>
#include <iostream>

using std::cout;
using std::endl;

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

        itsQbeam.resize(3);
        vector<string> beam=itsParset.getStringVector("restore.beam");
        CONRADCHECK(beam.size()==3, "Need three elements for beam");
        for (int i=0; i<3; i++)
        {
          casa::Quantity::read(itsQbeam(i), beam[i]);
        }

        /// Create the specified images from the definition in the
        /// parameter set. We can solve for any number of images
        /// at once (but you may/will run out of memory!)
        itsModel << itsParset.makeSubset("Images.");
        CONRADCHECK(itsModel, "Model not defined correctly");

        /// Create the solver from the parameterset definition and the existing
        /// definition of the parameters. 
        itsSolver=ImageSolverFactory::make(*itsModel, itsParset);
        CONRADCHECK(itsSolver, "Solver not defined correctly");
      }
      if (isWorker())
      {
        /// Get the list of measurement sets
        itsMs=itsParset.getStringVector("dataset");
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
            itsMs[i]=substituteWorkerNumber(tmpl);
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
      os() << "Calculating normal equations for " << ms << std::endl;
      // First time around we need to generate the equation 
      if ((!itsEquation)||discard)
      {
        os() << "Creating measurement equation" << std::endl;
        TableDataSource ds(ms);
        IDataSelectorPtr sel=ds.createSelector();
        sel << itsParset;
        IDataConverterPtr conv=ds.createConverter();
        conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
            "Hz");
        conv->setDirectionFrame(casa::MDirection::Ref(casa::MDirection::J2000));
        IDataSharedIter it=ds.createIterator(sel, conv);
        CONRADCHECK(itsModel, "Model not defined");
        CONRADCHECK(itsGridder, "Gridder not defined");
        itsEquation = conrad::scimath::Equation::ShPtr(new ImageFFTEquation (*itsModel, it, itsGridder));
      }
      else {
        os() << "Reusing measurement equation" << std::endl;
      }
      CONRADCHECK(itsEquation, "Equation not defined");
      CONRADCHECK(itsNe, "NormalEquations not defined");
      itsEquation->calcEquations(*itsNe);
      os() << "Calculated normal equations for "<< ms << " in "<< timer.real()
          << " seconds "<< std::endl;
    }

    /// Calculate the normal equations for a given measurement set
    void ImagerParallel::calcNE()
    {
      // Discard any old parameters
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
            calcOne(itsMs[iMs], itsMs.size()>1);
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
        os() << "Solving normal equations"<< std::endl;
        casa::Timer timer;
        timer.mark();
        Quality q;
        itsSolver->solveNormalEquations(q);
        os() << "Solved normal equations in "<< timer.real() << " seconds "
            << std::endl;
        *itsModel=itsSolver->parameters();
      }
    }

    /// Write the results out
    void ImagerParallel::writeModel()
    {
      if (isMaster())
      {
        os() << "Writing out results as CASA images"<< std::endl;
        vector<string> resultimages=itsModel->names();
        for (vector<string>::iterator it=resultimages.begin(); it
            !=resultimages.end(); it++)
        {
          os() << "Saving " << *it << " with name " << *it << std::endl;
          SynthesisParamsHelper::saveAsCasaImage(*itsModel, *it, *it);
        }

        if (itsRestore)
        {
          os() << "Writing out restored images as CASA images"<< std::endl;
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
            os() << "Saving restored image " << imageName << " with name "
                << imageName+string(".restored") << std::endl;
            SynthesisParamsHelper::saveAsCasaImage(*itsModel, imageName,
                imageName+string(".restored"));
          }
        }
      }
    }

  }
}
