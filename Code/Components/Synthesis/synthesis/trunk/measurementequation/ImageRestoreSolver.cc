#include <measurementequation/ImageRestoreSolver.h>
#include <measurementequation/SynthesisParamsHelper.h>

#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
CONRAD_LOGGER(logger, "");

#include <conrad/ConradError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <images/Images/TempImage.h>
#include <images/Images/Image2DConvolver.h>

using namespace conrad;
using namespace conrad::scimath;

#include <iostream>

#include <cmath>
using std::abs;

#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

namespace conrad
{
	namespace synthesis
	{

		ImageRestoreSolver::ImageRestoreSolver(const conrad::scimath::Params& ip,
		    const casa::Vector<casa::Quantum<double> >& beam) :
			ImageSolver(ip), itsBeam(beam)
		{
		}

		void ImageRestoreSolver::init()
		{
			resetNormalEquations();
		}

		// Solve for update simply by scaling the data vector by the diagonal term of the
		// normal equations i.e. the residual image
		bool ImageRestoreSolver::solveNormalEquations(
		    conrad::scimath::Quality& quality)
		{

			// Solving A^T Q^-1 V = (A^T Q^-1 A) P
			uint nParameters=0;

			// Find all the free parameters beginning with image
			vector<string> names(itsParams->completions("image"));
			map<string, uint> indices;

			for (vector<string>::const_iterator it=names.begin(); it!=names.end(); it++)
			{
				string name="image"+*it;
				if (itsParams->isFree(name))
				{
					indices[name]=nParameters;
					nParameters+=itsParams->value(name).nelements();
				}
			}
			CONRADCHECK(nParameters>0, "No free parameters in ImageRestoreSolver");

			for (map<string, uint>::const_iterator indit=indices.begin(); indit
			    !=indices.end(); indit++)
			{
                          CONRADLOG_INFO_STR(logger, "Restoring " << indit->first );

				// Axes are dof, dof for each parameter
				casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());

				CONRADCHECK(normalEquations().normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present");
				const casa::Vector<double>
				    & diag(normalEquations().normalMatrixDiagonal().find(indit->first)->second);
				CONRADCHECK(normalEquations().dataVector(indit->first).size()>0, "Data vector not present");
				const casa::Vector<double> &dv = normalEquations().dataVector(indit->first);
				double maxDiag(casa::max(diag));
				CONRADLOG_INFO_STR(logger, "Maximum of weights = " << maxDiag );
				double cutoff=tol()*maxDiag;

				// Create a temporary image
				boost::shared_ptr<casa::TempImage<float> > image(
				    SynthesisParamsHelper::tempImage(*itsParams, indit->first));

				casa::Image2DConvolver<float> convolver;
				const casa::IPosition pixelAxes(2, 0, 1);
				casa::LogIO logio;
				convolver.convolve(logio, *image, *image, casa::VectorKernel::GAUSSIAN,
				    pixelAxes, itsBeam, true, 1.0, false);
				SynthesisParamsHelper::update(*itsParams, indit->first, *image);

				// Add the residual image        
				{
					casa::Vector<double> value(itsParams->value(indit->first).reform(vecShape));
					for (uint elem=0; elem<dv.nelements(); elem++)
					{
						if (diag(elem)>cutoff)
						{
							value(elem)+=dv(elem)/diag(elem);
						}
						else
						{
							value(elem)+=dv(elem)/cutoff;
						}
					}
				}
			}

			quality.setDOF(nParameters);
			quality.setRank(0);
			quality.setCond(0.0);
			quality.setInfo("Restored image calculated");

			return true;
		}
		;

		Solver::ShPtr ImageRestoreSolver::clone() const
		{
			return Solver::ShPtr(new ImageRestoreSolver(*this));
		}

	}
}
