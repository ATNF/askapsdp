#include <measurementequation/ImageSolver.h>

#include <conrad/ConradError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

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

		ImageSolver::ImageSolver(const conrad::scimath::Params& ip) :
			conrad::scimath::Solver(ip)
		{
		}

		void ImageSolver::init()
		{
			itsNormalEquations.reset();
		}

		// Solve for update simply by scaling the data vector by the diagonal term of the
		// normal equations i.e. the residual image
		bool ImageSolver::solveNormalEquations(conrad::scimath::Quality& quality)
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
			CONRADCHECK(nParameters>0, "No free parameters in ImageSolver");

			//     	std::cout << "Solver parameters : " << *itsParams << std::endl;
			//     	std::cout << "Normal equation parameters : " << itsNormalEquations->parameters() << std::endl;

			for (map<string, uint>::const_iterator indit=indices.begin(); indit
			    !=indices.end(); indit++)
			{
				// Axes are dof, dof for each parameter
				casa::IPosition arrShape(itsParams->value(indit->first).shape());
				casa::IPosition vecShape(1, itsParams->value(indit->first).nelements());
				CONRADCHECK(itsNormalEquations->normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present for solution");
				const casa::Vector<double>
				    & diag(itsNormalEquations->normalMatrixDiagonal().find(indit->first)->second);
				CONRADCHECK(itsNormalEquations->dataVector().count(indit->first)>0, "Data vector not present for solution");
				const casa::Vector<double>& dv(itsNormalEquations->dataVector().find(indit->first)->second);
				double maxDiag(casa::max(diag));
				std::cout << "Maximum of weights = " << maxDiag << std::endl;
				double cutoff=tol()*maxDiag;
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
			quality.setInfo("Scaled residual calculated");

			/// Save the PSF and Weight
			saveWeights();
      savePSF();

			return true;
		}

		void ImageSolver::saveWeights()
		{
			// Save weights image
			vector<string> names(itsParams->completions("image"));
			for (vector<string>::const_iterator it=names.begin(); it!=names.end(); it++)
			{
				string name="image"+*it;
				if (itsNormalEquations->normalMatrixDiagonal().count(name))
				{
					const casa::IPosition arrShape(itsNormalEquations->shape().find(name)->second);
					Axes axes(itsParams->axes(name));
					string weightsName="weights"+*it;
					const casa::Array<double>
					    & ADiag(itsNormalEquations->normalMatrixDiagonal().find(name)->second.reform(arrShape));
					if(!itsParams->has(weightsName)) {
						itsParams->add(weightsName, ADiag, axes);			
					}
					else {
						itsParams->update(weightsName, ADiag);
					}
				}
			}
		}

		void ImageSolver::savePSF()
		{
			// Save weights image
			vector<string> names(itsParams->completions("image"));
			for (vector<string>::const_iterator it=names.begin(); it!=names.end(); it++)
			{
				string name="image"+*it;
				if (itsNormalEquations->normalMatrixSlice().count(name))
				{
					const casa::IPosition arrShape(itsNormalEquations->shape().find(name)->second);
					Axes axes(itsParams->axes(name));
					string psfName="psf"+*it;
					const casa::Array<double>
					    & APSF(itsNormalEquations->normalMatrixSlice().find(name)->second.reform(arrShape));
					if(!itsParams->has(psfName)) {
						itsParams->add(psfName, APSF, axes);
					}
					else {
						itsParams->update(psfName, APSF);
					}
				}
			}
		}

		Solver::ShPtr ImageSolver::clone() const
		{
			return Solver::ShPtr(new ImageSolver(*this));
		}

	}
}
