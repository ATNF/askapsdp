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
#include <stdexcept>

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
			resetNormalEquations();
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
				CONRADCHECK(normalEquations().normalMatrixDiagonal().count(indit->first)>0, "Diagonal not present for solution");
				const casa::Vector<double>
				    & diag(normalEquations().normalMatrixDiagonal().find(indit->first)->second);
				CONRADCHECK(normalEquations().dataVector(indit->first).size()>0, "Data vector not present for solution");
				const casa::Vector<double> &dv = normalEquations().dataVector(indit->first);
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
				if (normalEquations().normalMatrixDiagonal().count(name))
				{
					const casa::IPosition arrShape(normalEquations().shape().find(name)->second);
					Axes axes(itsParams->axes(name));
					string weightsName="weights"+*it;
					const casa::Array<double>
					    & ADiag(normalEquations().normalMatrixDiagonal().find(name)->second.reform(arrShape));
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
				if (normalEquations().normalMatrixSlice().count(name))
				{
					const casa::IPosition arrShape(normalEquations().shape().find(name)->second);
					Axes axes(itsParams->axes(name));
					string psfName="psf"+*it;
					const casa::Array<double>
					    & APSF(normalEquations().normalMatrixSlice().find(name)->second.reform(arrShape));
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

        /// @return a reference to normal equations object
        /// @note In this class and derived classes the type returned
        /// by this method is narrowed to always provide image-specific 
        /// normal equations objects
        const scimath::ImagingNormalEquations& ImageSolver::normalEquations() const
        {
           try {
              return dynamic_cast<const scimath::ImagingNormalEquations&>(
                                            Solver::normalEquations());
           }
           catch (const std::bad_cast &bc)
           {
              CONRADTHROW(ConradError, "An attempt to use incompatible normal "
                          "equations class with image solver");
           }
        }

	}
}
