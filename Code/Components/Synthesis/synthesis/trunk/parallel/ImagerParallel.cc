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

#include <measurementequation/ImageFFTEquation.h>
#include <measurementequation/SynthesisParamsHelper.h>
#include <measurementequation/ParsetInterface.h>

#include <measurementequation/ImageRestoreSolver.h>
#include <gridding/VisGridderFactory.h>

#include <fitting/ParamsCasaTable.h>
#include <fitting/Axes.h>

#include <dataaccess/DataAccessError.h>
#include <dataaccess/TableDataSource.h>
#include <dataaccess/ParsetInterface.h>

#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
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
		    const ParameterSet& parset) : SynParallel(argc, argv)
		{
			ParameterSet subset(itsParset.makeSubset("Cimager."));

			itsRestore=subset.getBool("restore", true);

			itsMs=parset.getStringVector("DataSet");

			itsQbeam.resize(3);
			vector<string> beam=subset.getStringVector("restore.beam");
			for (int i=0; i<3; i++)
			{
				casa::Quantity::read(itsQbeam(i), beam[i]);
			}

			/// Create the solver from the parameterset definition and the existing
			/// definition of the parameters. We create the solver here so that it
			/// can do any caching required
			itsSolver << subset;

			/// Create the gridder using a factory acting on a
			/// parameterset
			itsGridder=VisGridderFactory::make(subset);

		}
		;
		/// Calculate the normal equations for a given measurement set
		void ImagerParallel::calcNE(Params& model)
		{
			os() << "PREDIFFER Calculating normal equations for "<< itsMs[itsRank]
			    << std::endl;
			casa::Timer timer;
			timer.mark();
			TableDataSource ds(itsMs[itsRank]);
			IDataSelectorPtr sel=ds.createSelector();
			sel << itsParset;
			IDataConverterPtr conv=ds.createConverter();
			conv->setFrequencyFrame(casa::MFrequency::Ref(casa::MFrequency::TOPO),
			    "Hz");
			IDataSharedIter it=ds.createIterator(sel, conv);
			it.init();
			it.chooseOriginal();
			ImageFFTEquation ie(model, it, itsGridder);
			ie.calcEquations(itsNe);
			os() << "PREDIFFER Calculated normal equations for "<< itsMs[itsRank]
			    << " in "<< timer.real() << " seconds "<< std::endl;
		}

		void ImagerParallel::solveNE(Params& model)
		{
			// Receive the normal equations - no-op if serial
			receiveNE();
			os() << "SOLVER Solving normal equations"<< std::endl;
			casa::Timer timer;
			timer.mark();
			Quality q;
			itsSolver->solveNormalEquations(q);
			os() << "SOLVER Solved normal equations in "<< timer.real()
			    << " seconds "<< std::endl;
			model=itsSolver->parameters();
			// Broadcast the model - no-op if serial
			broadcastModel(model);
		}

		/// Write the results out
		void ImagerParallel::writeResults(Params& model)
		{
			os() << "Writing out results as CASA images"<< std::endl;
			vector<string> resultimages=model.names();
			for (vector<string>::iterator it=resultimages.begin(); it
			    !=resultimages.end(); it++)
			{
				SynthesisParamsHelper::saveAsCasaImage(model, *it, *it);
			}

			if (itsRestore)
			{
				os() << "Writing out restored images as CASA images"<< std::endl;
				ImageRestoreSolver ir(model, itsQbeam);
				ir.setThreshold(itsSolver->threshold());
				ir.setVerbose(itsSolver->verbose());
				ir.copyNormalEquations(*itsSolver);
				Quality q;
				ir.solveNormalEquations(q);
				resultimages=ir.parameters().names();
				for (vector<string>::iterator it=resultimages.begin(); it
				    !=resultimages.end(); it++)
				{
					SynthesisParamsHelper::saveAsCasaImage(model, *it, *it
					    +string(".restored"));
				}
			}
		}

	}
}