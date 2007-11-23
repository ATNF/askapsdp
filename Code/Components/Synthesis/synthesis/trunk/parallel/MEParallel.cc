/// @file
///
/// @brief Base class for parallel synthesis applications
/// @details
/// Supports parallel algorithms by providing methods for initialization
/// of MPI connections, sending normal equations and models around.
/// There is assumed to be one solver and many prediffers.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
/// 

#include <Blob/BlobString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

#include <casa/OS/Timer.h>

#include <conrad/ConradError.h>

#include <parallel/MEParallel.h>
#include <fitting/ImagingNormalEquations.h>

using namespace conrad;
using namespace conrad::cp;
using namespace conrad::scimath;

namespace conrad
{
	namespace synthesis
	{

		MEParallel::MEParallel(int argc, const char** argv) : SynParallel(argc, argv)
		{
			itsSolver = Solver::ShPtr(new Solver(*itsModel));
			itsEquation = Equation::ShPtr(new Equation(*itsModel));
			itsNe = ImagingNormalEquations::ShPtr(new ImagingNormalEquations(*itsModel));

		}

		MEParallel::~MEParallel()
		{
		}

		// Send the normal equations from this worker to the master as a blob
		void MEParallel::sendNE()
		{
			if (isParallel()&&isWorker())
			{
				casa::Timer timer;
				timer.mark();
				os() << "Sending normal equations to the solver via MPI" << std::endl;

				LOFAR::BlobString bs;
				bs.resize(0);
				LOFAR::BlobOBufString bob(bs);
				LOFAR::BlobOStream out(bob);
				out.putStart("ne", 1);
				out << itsRank << *itsNe;
				out.putEnd();
				itsConnectionSet->write(0, bs);
				os() << "Sent normal equations to the solver via MPI in "
				    << timer.real()<< " seconds "<< std::endl;
			}
		}

		// Receive the normal equations as a blob
		void MEParallel::receiveNE()
		{
			CONRADCHECK(itsSolver, "Solver not yet defined");
			if (isParallel()&&isMaster())
			{
				os() << "Initialising solver"<< std::endl;
				itsSolver->init();
				itsSolver->setParameters(*itsModel);
				
				os() << "Waiting for normal equations"<< std::endl;
				casa::Timer timer;
				timer.mark();

				LOFAR::BlobString bs;
				int rank;

				for (int i=1; i<itsNNode; i++)
				{
					itsConnectionSet->read(i-1, bs);
					LOFAR::BlobIBufString bib(bs);
					LOFAR::BlobIStream in(bib);
					int version=in.getStart("ne");
					CONRADASSERT(version==1);
					in >> rank >> *itsNe;
					in.getEnd();
					itsSolver->addNormalEquations(*itsNe);
					os() << "Received normal equations from prediffer "<< rank
					    << " after "<< timer.real() << " seconds"<< std::endl;
				}
				os()<< "Received normal equations from all prediffers via MPI in "
				    << timer.real() << " seconds"<< std::endl;
			}
		}

		void MEParallel::writeModel()
		{
		}

	}
}
