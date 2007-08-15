///
/// @file : Synthesis imaging program
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
#include <measurementequation/ParsetInterface.h>

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
		    const ParameterSet& parset) :
			SynParallel(argc, argv), itsParset(parset)
		{

			/// Create the specified images from the definition in the
			/// parameter set. We can solve for any number of images
			/// at once (but you may/will run out of memory!)
			itsModel << parset.makeSubset("Images.");

			ParameterSet subset(itsParset.makeSubset("Cimager."));

			if (isSolver())
			{
				itsRestore=itsParset.getBool("Cimager.restore", true);

				itsQbeam.resize(3);
				vector<string> beam=itsParset.getStringVector("Cimager.restore.beam");
				for (int i=0; i<3; i++)
				{
					casa::Quantity::read(itsQbeam(i), beam[i]);
				}

				/// Create the solver from the parameterset definition and the existing
				/// definition of the parameters. 
				itsSolver=ImageSolverFactory::make(*itsModel, subset);
				CONRADCHECK(itsSolver, "Solver not defined correctly");
			}
			if(isPrediffer())
			{
				/// Get the list of measurement sets
				itsMs=itsParset.getStringVector("DataSet");
				if(itsNNode>1) {
				  CONRADCHECK(itsMs.size()==(itsNNode-1), "When running in parallel, need one data set per node");
				}

				/// Create the gridder using a factory acting on a
				/// parameterset
				itsGridder=VisGridderFactory::make(subset);
				CONRADCHECK(itsGridder, "Gridder not defined correctly");
			}
		}

		void ImagerParallel::calcOne(const string& ms)
		{
			casa::Timer timer;
			timer.mark();
			os() << "Calculating normal equations for "
			    << ms << std::endl;
			TableDataSource ds(ms);
			IDataSelectorPtr sel=ds.createSelector();
			sel << itsParset;
			IDataConverterPtr conv=ds.createConverter();
			conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO), "Hz");
			IDataSharedIter it=ds.createIterator(sel, conv);
			ImageFFTEquation ie(*itsModel, it, itsGridder);
			ie.calcEquations(*itsNe);
			os() << "Calculated normal equations for "<< ms
			    << " in "<< timer.real() << " seconds "<< std::endl;
		}

		/// Calculate the normal equations for a given measurement set
		void ImagerParallel::calcNE()
		{
			if (isPrediffer())
			{
				CONRADCHECK(itsGridder, "Gridder not defined");
				CONRADCHECK(itsModel, "Model not defined");
				CONRADCHECK(itsMs.size()>0, "Data sets not defined");
				// Discard any old parameters
				itsNe=NormalEquations::ShPtr(new NormalEquations(*itsModel));
				
				CONRADCHECK(itsNe, "NormalEquations not defined");
				
				if (isParallel())
				{
					calcOne(itsMs[itsRank-1]);
					sendNE();
				}
				else
				{
					for (int iMs=0; iMs<itsMs.size(); iMs++)
					{
						calcOne(itsMs[iMs]);
						CONRADCHECK(itsSolver, "Solver not defined correctly");
						itsSolver->setParameters(*itsModel);
						itsSolver->addNormalEquations(*itsNe);
					}
				}
			}
		}

		void ImagerParallel::solveNE()
		{
			if (isSolver())
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
				os() << "Solved normal equations in "<< timer.real()
				    << " seconds "<< std::endl;
				*itsModel=itsSolver->parameters();
			}
		}

		/// Write the results out
		void ImagerParallel::writeModel()
		{
			if (isSolver())
			{
				os() << "Writing out results as CASA images"<< std::endl;
				vector<string> resultimages=itsModel->names();
				for (vector<string>::iterator it=resultimages.begin(); it
				    !=resultimages.end(); it++)
				{
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
					resultimages=ir.parameters().names();
					for (vector<string>::iterator it=resultimages.begin(); it
					    !=resultimages.end(); it++)
					{
						SynthesisParamsHelper::saveAsCasaImage(*itsModel, *it, *it
						    +string(".restored"));
					}
				}
			}
		}

	}
}
